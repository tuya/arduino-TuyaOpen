# 基础 HTTPS 客户端示例

## 功能描述

此示例演示如何使用带 SSL/TLS 证书验证的 HTTPClient 库发送安全的 HTTPS GET 请求。它连接到 WiFi 网络并向安全的 Web 服务器发送经过身份验证的 HTTPS 请求。

该示例包含用于 SSL/TLS 验证的根 CA 证书，确保与远程服务器的安全通信。非常适合学习如何与安全的 Web 服务和 API 交互。

## 硬件要求

- 具有 WiFi 功能和 SSL/TLS 支持的涂鸦开放平台开发板（ESP32、T2、T3、T5 等）
- 活动的 WiFi 网络连接
- 足够的闪存/RAM 用于 SSL/TLS 操作

## 使用说明

1. 修改代码中的 WiFi 凭据：
   ```cpp
   WiFi.begin("your_ssid", "your_passwd");
   ```
   将 `"your_ssid"` 和 `"your_passwd"` 替换为您实际的 WiFi 网络凭据。

2. （可选）如果连接到使用不同证书颁发机构的其他服务器，请更新根 CA 证书。

3. （可选）更改目标 URL：
   ```cpp
   http.begin("https://httpbin.org/get");
   ```

4. 将程序上传到开发板。

5. 打开串口监视器（波特率 115200）查看连接状态和 HTTPS 响应。

6. 开发板将连接到 WiFi 并每 5 秒发送一次 HTTPS GET 请求。

## 功能要点

- **安全 HTTPS 连接**：使用 SSL/TLS 加密进行安全通信
- **证书验证**：包含 Amazon 根 CA 证书用于验证
- **自定义头部**：添加 HTTP 头部（Content-Type: application/json）
- **响应处理**：接收并显示 HTTPS 响应正文
- **错误处理**：对连接和请求失败进行全面的错误检查
- **调试日志**：使用 Log 库的 DEBUG 级别进行详细输出

## 代码要点

- 包含 Amazon 根 CA 证书（RSA 2048 M02）用于 SSL/TLS 验证
- 使用 `Log.setLevel(LogClass::DEBUG)` 进行详细日志记录
- 等待 WiFi 连接：`WiFi.status() == WSS_GOT_IP`
- 将 CA 证书传递给 GET 请求进行验证：
  ```cpp
  http.GET(headers, headers_length, (uint8_t *)rootCACertificate, cacert_len, &http_response)
  ```
- 证书长度包括空终止符：`strlen(rootCACertificate) + 1`
- 显示来自安全连接的响应内容
- 使用以下方法正确释放资源：`http.end(&http_response)`
- 包含 4 秒启动延迟和 2 秒 WiFi 连接延迟
