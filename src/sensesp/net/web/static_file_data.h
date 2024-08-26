#ifndef SENSESP_NET_WEB_STATIC_FILE_DATA_H
#define SENSESP_NET_WEB_STATIC_FILE_DATA_H

namespace sensesp {

const char kContentType[] = "Content-Type";
const char kContentEncoding[] = "Content-Encoding";

struct StaticFileData {
  const char* url;
  const char* content;
  const unsigned int content_length;
  const char* content_type;
  const char* content_encoding;
};

}  // namespace sensesp

#endif  // SENSESP_NET_WEB_STATIC_PAGE_DATA_H
