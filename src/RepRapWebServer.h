/*
  ESP8266WebServer.h - Dead simple web-server.
  Supports only one simultaneous client, knows how to handle GET and POST.

  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  Modified 8 May 2015 by Hristo Gochkov (proper post and file upload handling)
*/


#ifndef REPRAPWEBSERVER_H
#define REPRAPWEBSERVER_H

#include <functional>

enum HTTPMethod { HTTP_ANY, HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_PATCH, HTTP_DELETE, HTTP_OPTIONS };
enum HTTPUploadStatus { UPLOAD_FILE_START, UPLOAD_FILE_WRITE, UPLOAD_FILE_END, UPLOAD_FILE_ABORTED };

#define HTTP_DOWNLOAD_UNIT_SIZE 1460
#define HTTP_UPLOAD_BUFLEN 2048
#define HTTP_MAX_DATA_WAIT 1000 //ms to wait for the client to send the request
#define HTTP_MAX_CLOSE_WAIT 2000 //ms to wait for the client to close the connection

#define CONTENT_LENGTH_UNKNOWN ((size_t) -1)
#define CONTENT_LENGTH_NOT_SET ((size_t) -2)

class RepRapWebServer;

typedef struct {
  HTTPUploadStatus status;
  String  filename;
  String  name;
  String  type;
  size_t  totalSize;    // file size
  size_t  currentSize;  // size of data currently in buf
  uint8_t buf[HTTP_UPLOAD_BUFLEN];
} HTTPUpload;

#include "RequestHandler.h"

namespace fs {
class FS;
}

class RepRapWebServer
{
public:
  RepRapWebServer(IPAddress addr, int port = 80);
  RepRapWebServer(int port = 80);
  ~RepRapWebServer();

  void begin();
  void handleClient();

  typedef std::function<void(void)> THandlerFunction;
  void on(const char* uri, THandlerFunction handler);
  void on(const char* uri, HTTPMethod method, THandlerFunction fn);
  void on(const char* uri, HTTPMethod method, THandlerFunction fn, THandlerFunction ufn);
  void onPrefix(const char* prefix, HTTPMethod method, RepRapWebServer::THandlerFunction fn, RepRapWebServer::THandlerFunction ufn);
  void addHandler(RequestHandler* handler);
  void serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_header = NULL );
  void onNotFound(THandlerFunction fn);  //called when handler is not assigned
  void onFileUpload(THandlerFunction fn); //handle file uploads

  String uri() { return _currentUri; }
  String fullUri() { return _currentFullUri; }
  HTTPMethod method() { return _currentMethod; }
  WiFiClient& client() { return _currentClient; }
  HTTPUpload& upload() { return _currentUpload; }

  String arg(const char* name);   // get request argument value by name
  String arg(int i);              // get request argument value by number
  String argName(int i);          // get request argument name by number
  int args();                     // get arguments count
  bool hasArg(const char* name);  // check if argument exists
  void collectHeaders(const char* headerKeys[], const size_t headerKeysCount); // set the request headers to collect
  String header(const char* name);   // get request header value by name
  String header(int i);              // get request header value by number
  String headerName(int i);          // get request header name by number
  int headers();                     // get header count
  bool hasHeader(const char* name);  // check if header exists

  String hostHeader();            // get request host header if available or empty String if not

  // send response to the client
  // code - HTTP response code, can be 200 or 404
  // content_type - HTTP content type, like "text/plain" or "image/png"
  // content - actual content body
  void send(int code, size_t contentLength, const __FlashStringHelper *contentType, const uint8_t *data, size_t dataLength, bool isLast);

  void send(int code, const char* content_type = NULL, const String& content = String(""));
  void send(int code, char* content_type, const String& content);
  void send(int code, const String& content_type, const String& content);

  void setContentLength(size_t contentLength) { _contentLength = contentLength; }
  void sendHeader(const String& name, const String& value, bool first = false);
  void sendContent(const uint8_t *content, size_t dataLength, bool last);
  void sendContent(const String& content, bool last = true);

  void servePrinter(bool b) { _servingPrinter = b; }
  uint32_t getPostLength() const { return _postLength; }

template<typename T> size_t streamFile(T &file, const String& contentType){
  setContentLength(file.size());
  if (String(file.name()).endsWith(".gz") &&
      contentType != "application/x-gzip" &&
      contentType != "application/octet-stream"){
    sendHeader("Content-Encoding", "gzip");
  }
  send(200, contentType, "");
  return _currentClient.write(file, HTTP_DOWNLOAD_UNIT_SIZE);
}

  size_t readPostdata(WiFiClient& client, uint8_t *buffer, size_t buflen);

protected:
  void _addRequestHandler(RequestHandler* handler);
  void _handleRequest();
  bool _parseRequest(WiFiClient& client, uint32_t& postLength);
  void _parseArguments(String data);
  static const char* _responseCodeToString(int code);
  bool _parseForm(WiFiClient& client, String boundary, uint32_t len);
  bool _parseFormUploadAborted();
  void _uploadWriteByte(uint8_t b);
  uint8_t _uploadReadByte(WiFiClient& client);
  void _prepareHeader(String& response, int code, const char* content_type, size_t contentLength);
  bool _collectHeader(const char* headerName, const char* headerValue);
  String urlDecode(const String& text);

  struct RequestArgument {
    String key;
    String value;
  };

  WiFiServer  _server;

  WiFiClient  _currentClient;
  HTTPMethod  _currentMethod;
  String      _currentUri;
  String      _currentFullUri;

  RequestHandler*  _currentHandler;
  RequestHandler*  _firstHandler;
  RequestHandler*  _lastHandler;
  THandlerFunction _notFoundHandler;
  THandlerFunction _fileUploadHandler;

  int              _currentArgCount;
  RequestArgument* _currentArgs;
  HTTPUpload       _currentUpload;

  int              _headerKeysCount;
  RequestArgument* _currentHeaders;
  size_t           _contentLength;
  String           _responseHeaders;

  String           _hostHeader;

  uint32_t _postLength;
  bool _servingPrinter;
};


#endif //ESP8266WEBSERVER_H
