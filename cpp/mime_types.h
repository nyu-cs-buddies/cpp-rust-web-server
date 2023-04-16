#include <string>
#include <unordered_map>

namespace MIME {
  const std::unordered_map<std::string, std::string> mime_types = {
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".bmp", "image/bmp"},
    {".ico", "image/x-icon"},
    {".svg", "image/svg+xml"},
    {".css", "text/css"},
    {".html", "text/html"},
    {".htm", "text/html"},
    {".txt", "text/plain"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".xml", "application/xml"},
    {".pdf", "application/pdf"},
    {".zip", "application/zip"},
    {".gz", "application/gzip"},
    {".tar", "application/x-tar"},
    {".wav", "audio/wav"},
    {".mp3", "audio/mpeg"},
    {".mp4", "video/mp4"},
    {".avi", "video/x-msvideo"},
    {".doc", "application/msword"},
    {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".ppt", "application/vnd.ms-powerpoint"},
    {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {".xls", "application/vnd.ms-excel"},
    {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {".eot", "application/vnd.ms-fontobject"},
    {".otf", "application/font-sfnt"},
    {".ttf", "application/font-sfnt"},
    {".woff", "application/font-woff"},
    {".woff2", "application/font-woff2"},
    {".7z", "application/x-7z-compressed"},
    {".rar", "application/x-rar-compressed"},
    {".rtf", "application/rtf"},
    {".wasm", "application/wasm"},
    {".webp", "image/webp"},
    {".aac", "audio/aac"},
    {".abw", "application/x-abiword"},
    {".arc", "application/x-freearc"},
    {".avi", "video/x-msvideo"},
    {".azw", "application/vnd.amazon.ebook"},
    {".bin", "application/octet-stream"},
    {".bz", "application/x-bzip"}
  };
}