#include "DNSServer.h"
#include <lwip/def.h>
#include <Arduino.h>

DNSServer::DNSServer()
{
  _ttl = htonl(DNS_DEFAULT_TTL);
  _ErrReplyCode = DNSReplyCode::NonExistentDomain;
  _DnsHeader    = (DNSHeader*) malloc( sizeof(DNSHeader) ) ;
  _DnsQuestion  = (DNSQuestion*) malloc( sizeof(DNSQuestion) ) ;     
  _buf     = NULL;
  _CurPacketSize = 0;
  _port = 0;
}

DNSServer::~DNSServer()
{
  if (_DnsHeader) {
    free(_DnsHeader);
    _DnsHeader = NULL;
  }
  if (_DnsQuestion) {
    free(_DnsQuestion);
    _DnsQuestion = NULL;
  }
  if (_buf) {
    free(_buf);
    _buf = NULL;
  }
}

bool DNSServer::start(const uint16_t &port, const String &domainName,
                     const IPAddress &resolvedIP)
{
  _port = port;
  _buf = NULL;
  _domain = domainName;
  _ResolveIP[0] = resolvedIP[0];
  _ResolveIP[1] = resolvedIP[1];
  _ResolveIP[2] = resolvedIP[2];
  _ResolveIP[3] = resolvedIP[3];
  downcaseAndRemoveWwwPrefix(_domain);
  return _Udp.begin(_port) == 1;
}

void DNSServer::setErrorReplyCode(const DNSReplyCode &replyCode)
{
  _ErrReplyCode = replyCode;
}

void DNSServer::setTTL(const uint32_t &ttl)
{
  _ttl = htonl(ttl);
}

void DNSServer::stop()
{
  _Udp.stop();
  free(_buf);
  _buf = NULL;
}

void DNSServer::downcaseAndRemoveWwwPrefix(String &domainName)
{
  domainName.toLowerCase();
  domainName.replace("www.", "");
}

void DNSServer::processNextRequest()
{
  _CurPacketSize = _Udp.parsePacket();
  if (_CurPacketSize)
  {
    bool parse_ok = true;
    // Allocate buffer for the DNS query
    if (_buf != NULL) 
      free(_buf);
    _buf = (unsigned char*)malloc(_CurPacketSize);
    if (_buf == NULL) 
      return;

    // Put the packet received in the buffer and get DNS header (beginning of message)
    // and the question
    _Udp.read(_buf, _CurPacketSize);
    
    // Check if packet is large enough to contain DNS header
    if (_CurPacketSize < DNS_HEADER_SIZE) {
      parse_ok = false;
    }
    
    if (parse_ok) {
      memcpy( _DnsHeader, _buf, DNS_HEADER_SIZE );
    }
    
    if ( parse_ok && requestIncludesOnlyOneQuestion() ) {
      if (_CurPacketSize < DNS_HEADER_SIZE + 1) {
        parse_ok = false;
      }

      if (parse_ok) {
        // The QName has a variable length, maximum 255 bytes and is comprised of multiple labels.
        // Each label contains a byte to describe its length and the label itself. The list of 
        // labels terminates with a zero-valued byte. In "github.com", we have two labels "github" & "com"
        // Iterate through the labels and copy them as they come into a single buffer (for simplicity's sake)
        _DnsQuestion->QNameLength = 0 ;
        while ( parse_ok && _DnsQuestion->QNameLength < sizeof(_DnsQuestion->QName) )
        {
          uint16_t q_off = _DnsQuestion->QNameLength;
          uint16_t src_off = DNS_HEADER_SIZE + q_off;

          // Check if we can read the length byte or terminator
          if (src_off >= _CurPacketSize) {
            parse_ok = false;
            break;
          }

          // Check for terminator
          if (_buf[src_off] == 0) {
            break;
          }

          uint16_t label_len = _buf[src_off];
          
          if ((uint32_t)src_off + (uint32_t)label_len + 1u > (uint32_t)_CurPacketSize) {
            parse_ok = false;
            break;
          }

          // Keep room for the final trailing '\0'
          if ((uint32_t)q_off + (uint32_t)label_len + 1u >= sizeof(_DnsQuestion->QName)) {
            parse_ok = false;
            break;
          }

          memcpy((void*)&_DnsQuestion->QName[q_off], (void*)&_buf[src_off], label_len + 1);
          _DnsQuestion->QNameLength += label_len + 1;
        }

        if (parse_ok && _DnsQuestion->QNameLength < sizeof(_DnsQuestion->QName)) {
          _DnsQuestion->QName[_DnsQuestion->QNameLength] = 0;
          _DnsQuestion->QNameLength++;

          // Copy the QType and QClass
          uint32_t qtype_off = DNS_HEADER_SIZE + _DnsQuestion->QNameLength;
          uint32_t qclass_off = qtype_off + sizeof(_DnsQuestion->QType);
          if (qclass_off + sizeof(_DnsQuestion->QClass) > (uint32_t)_CurPacketSize) {
            parse_ok = false;
          } else {
            memcpy(&_DnsQuestion->QType, (void*)&_buf[qtype_off], sizeof(_DnsQuestion->QType));
            memcpy(&_DnsQuestion->QClass, (void*)&_buf[qclass_off], sizeof(_DnsQuestion->QClass));
          }
        }
      }
    }
    

    if (parse_ok &&
        _DnsHeader->QRFlag == DNS_QR_QUERY &&
        _DnsHeader->OPCode == DNS_OPCODE_QUERY &&
        requestIncludesOnlyOneQuestion() &&
        (_domain == "*" || getDomainNameWithoutWwwPrefix() == _domain)
       )
    {
      replyWithIP();
    }
    else if (parse_ok && _DnsHeader->QRFlag == DNS_QR_QUERY)
    {
      replyWithCustomCode();
    }

    free(_buf);
    _buf = NULL;
  }
}

bool DNSServer::requestIncludesOnlyOneQuestion()
{
  return ntohs(_DnsHeader->QuesCnt) == 1 &&
         _DnsHeader->AnsCnt == 0 &&
         _DnsHeader->AuthCnt == 0 &&
         _DnsHeader->ResCnt == 0;
}


String DNSServer::getDomainNameWithoutWwwPrefix()
{
  // Error checking : if the buffer containing the DNS request is a null pointer, return an empty domain
  String parsedDomainName = "";
  if (_buf == NULL) 
    return parsedDomainName;
  
  // Check if packet is large enough to contain domain name
  if (_CurPacketSize <= DNS_OFFSET_DOMAIN_NAME)
    return parsedDomainName;
  
  // Set the start of the domain just after the header (12 bytes). If equal to null character, return an empty domain
  unsigned char *start = _buf + DNS_OFFSET_DOMAIN_NAME;
  if (*start == 0)
  {
    return parsedDomainName;
  }

  int pos = 0;
  int maxLen = _CurPacketSize - DNS_OFFSET_DOMAIN_NAME;
  
  while(true)
  {
    // Check if we can read the label length byte
    if (pos >= maxLen) {
      return "";  // Invalid packet
    }
    
    unsigned char labelLength = *(start + pos);
    
    // Check for terminator
    if (labelLength == 0) {
      downcaseAndRemoveWwwPrefix(parsedDomainName);
      return parsedDomainName;
    }
    
    // Validate label length (DNS labels are max 63 bytes)
    if (labelLength > 63 || pos + labelLength >= maxLen) {
      return "";  // Invalid packet
    }
    
    // Read the label
    for(int i = 0; i < labelLength; i++)
    {
      pos++;
      if (pos >= maxLen) {
        return "";  // Invalid packet
      }
      parsedDomainName += (char)*(start + pos);
    }
    pos++;
    
    // Check if we can read the next byte (either next label length or terminator)
    if (pos >= maxLen) {
      return "";  // Invalid packet
    }
    
    if (*(start + pos) == 0)
    {
      downcaseAndRemoveWwwPrefix(parsedDomainName);
      return parsedDomainName;
    }
    else
    {
      parsedDomainName += ".";
    }
  }
}

void DNSServer::replyWithIP()
{
  _Udp.beginPacket(_Udp.remoteIP(), _Udp.remotePort());
  
  // Change the type of message to a response and set the number of answers equal to 
  // the number of questions in the header
  _DnsHeader->QRFlag      = DNS_QR_RESPONSE;
  _DnsHeader->AnsCnt = _DnsHeader->QuesCnt;
  _Udp.write( (unsigned char*) _DnsHeader, DNS_HEADER_SIZE ) ;

  // Write the question
  _Udp.write(_DnsQuestion->QName, _DnsQuestion->QNameLength) ;
  _Udp.write( (unsigned char*) &_DnsQuestion->QType, 2 ) ;
  _Udp.write( (unsigned char*) &_DnsQuestion->QClass, 2 ) ;

  // Write the answer 
  // Use DNS name compression : instead of repeating the name in this RNAME occurrence,
  // set the two MSB of the byte corresponding normally to the length to 1. The following
  // 14 bits must be used to specify the offset of the domain name in the message 
  // (<255 here so the first byte has the 6 LSB at 0) 
  _Udp.write((uint8_t) 0xC0); 
  _Udp.write((uint8_t) DNS_OFFSET_DOMAIN_NAME);  

  // DNS type A : host address, DNS class IN for INternet, returning an IPv4 address 
  uint16_t answerType = htons(DNS_TYPE_HOST_ADDRESS), answerClass = htons(DNS_CLASS_IN), answerIPv4 = htons(DNS_RDLENGTH_IPV4)  ; 
  _Udp.write((unsigned char*) &answerType, 2 );
  _Udp.write((unsigned char*) &answerClass, 2 );
  _Udp.write((unsigned char*) &_ttl, 4);        // DNS Time To Live
  _Udp.write((unsigned char*) &answerIPv4, 2 );
  _Udp.write(_ResolveIP, sizeof(_ResolveIP)); // The IP address to return
  _Udp.endPacket();

}

void DNSServer::replyWithCustomCode()
{
  _DnsHeader->QRFlag = DNS_QR_RESPONSE;
  _DnsHeader->RspCode = (unsigned char)_ErrReplyCode;
  _DnsHeader->QuesCnt = 0;

  _Udp.beginPacket(_Udp.remoteIP(), _Udp.remotePort());
  _Udp.write((unsigned char*)_DnsHeader, sizeof(DNSHeader));
  _Udp.endPacket();
}
