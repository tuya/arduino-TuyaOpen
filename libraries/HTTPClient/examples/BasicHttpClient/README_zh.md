# 基础 HTTP 客户端示例

## 功能描述

此示例演示如何在涂鸦开放平台上使用 HTTPClient 库发送 HTTP GET 请求。它连接到 WiFi 网络并定期向 Web 服务器发送 HTTP GET 请求，在串口监视器中显示响应内容。

该示例对于学习如何通过 HTTP 协议与 Web 服务和 API 交互非常有用。

## 硬件要求

- 具有 WiFi 功能的涂鸦开放平台开发板（ESP32、T2、T3、T5 等）
- 活动的 WiFi 网络连接

## 使用说明

1. 修改代码中的 WiFi 凭据：
   ```cpp
   WiFi.begin("your_ssid", "your_passwd");
   ```
   将 `"your_ssid"` 和 `"your_passwd"` 替换为您实际的 WiFi 网络凭据。

2. （可选）更改目标 URL：
   ```cpp
   http.begin("http://www.moe.gov.cn/jyb_sjzl/");
   ```

3. 将程序上传到开发板。

4. 打开串口监视器（波特率 115200）查看连接状态和 HTTP 响应。

5. 开发板将连接到 WiFi 并每 5 秒发送一次 HTTP GET 请求。

## 功能要点

- **WiFi 连接**：自动连接到指定的 WiFi 网络
- **HTTP GET 请求**：使用自定义头部发送 GET 请求
- **响应处理**：接收并显示 HTTP 响应正文
- **自定义头部**：演示添加 HTTP 头部（Content-Type: application/json）
- **错误处理**：检查连接和请求错误
- **定期请求**：每 5 秒重复请求

## 代码要点

- 使用 `HTTPClient` 类进行 HTTP 操作
- 在发送请求前检查 WiFi 连接状态：`WiFi.status() == WSS_GOT_IP`
- 使用 `http_client_header_t` 结构配置自定义 HTTP 头部
- 使用以下方法发送 GET 请求：`http.GET(headers, headers_length, NULL, 0, &http_response)`
- 通过串口显示 HTTP 响应正文内容
- 使用以下方法正确关闭 HTTP 连接：`http.end(&http_response)`
- 包含 4 秒启动延迟以进行系统初始化
