/*
 * ============================================================
 *  Piscok Market — Text Case Generator Dashboard
 *  Web server on http://localhost:8080
 *  Build:  g++ -o proyek proyek.cpp -lws2_32   (Windows)
 *          g++ -o proyek proyek.cpp             (Linux/macOS)
 * ============================================================
 */

#ifdef _WIN32
  #define _WIN32_WINNT 0x0601
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef int socklen_t;
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #define SOCKET int
  #define INVALID_SOCKET (-1)
  #define closesocket close
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <map>
#include <vector>
#include <cstring>

// ─── Text-case transformations ──────────────────────────────────────────────

std::string toUpperCase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

std::string toLowerCase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::string toTitleCase(const std::string& s) {
    std::string result = s;
    bool newWord = true;
    for (char& c : result) {
        if (std::isspace(c)) { newWord = true; }
        else if (newWord)    { c = (char)std::toupper(c); newWord = false; }
        else                 { c = (char)std::tolower(c); }
    }
    return result;
}

std::string toSentenceCase(const std::string& s) {
    std::string result = toLowerCase(s);
    bool cap = true;
    for (char& c : result) {
        if (cap && std::isalpha(c)) { c = (char)std::toupper(c); cap = false; }
        if (c == '.' || c == '!' || c == '?') cap = true;
    }
    return result;
}

std::string toCamelCase(const std::string& s) {
    std::string result;
    bool nextUpper = false;
    bool first = true;
    for (char c : s) {
        if (std::isspace(c) || c == '_' || c == '-') {
            nextUpper = !first;
        } else {
            if (nextUpper) { result += (char)std::toupper(c); nextUpper = false; }
            else           { result += (char)std::tolower(c); }
            first = false;
        }
    }
    return result;
}

std::string toPascalCase(const std::string& s) {
    std::string result;
    bool nextUpper = true;
    for (char c : s) {
        if (std::isspace(c) || c == '_' || c == '-') { nextUpper = true; }
        else {
            if (nextUpper) { result += (char)std::toupper(c); nextUpper = false; }
            else           { result += (char)std::tolower(c); }
        }
    }
    return result;
}

std::string toSnakeCase(const std::string& s) {
    std::string lower = toLowerCase(s);
    std::string result;
    for (char c : lower) {
        result += (std::isspace(c) || c == '-') ? '_' : c;
    }
    return result;
}

std::string toKebabCase(const std::string& s) {
    std::string lower = toLowerCase(s);
    std::string result;
    for (char c : lower) {
        result += (std::isspace(c) || c == '_') ? '-' : c;
    }
    return result;
}

std::string toAlternatingCase(const std::string& s) {
    std::string result = s;
    bool upper = true;
    for (char& c : result) {
        if (std::isalpha(c)) {
            c = upper ? (char)std::toupper(c) : (char)std::tolower(c);
            upper = !upper;
        }
    }
    return result;
}

std::string toInverseCase(const std::string& s) {
    std::string result = s;
    for (char& c : result) {
        if      (std::isupper(c)) c = (char)std::tolower(c);
        else if (std::islower(c)) c = (char)std::toupper(c);
    }
    return result;
}

// ─── URL decode ─────────────────────────────────────────────────────────────

std::string urlDecode(const std::string& src) {
    std::string result;
    for (size_t i = 0; i < src.size(); ++i) {
        if (src[i] == '+') { result += ' '; }
        else if (src[i] == '%' && i + 2 < src.size()) {
            int val;
            std::istringstream iss(src.substr(i + 1, 2));
            if (iss >> std::hex >> val) { result += (char)val; i += 2; }
            else { result += src[i]; }
        } else { result += src[i]; }
    }
    return result;
}

// ─── Parse POST body  key=value&key=value ───────────────────────────────────

std::map<std::string, std::string> parseForm(const std::string& body) {
    std::map<std::string, std::string> params;
    std::istringstream ss(body);
    std::string token;
    while (std::getline(ss, token, '&')) {
        auto eq = token.find('=');
        if (eq != std::string::npos)
            params[urlDecode(token.substr(0, eq))] = urlDecode(token.substr(eq + 1));
    }
    return params;
}

// ─── HTML escape ────────────────────────────────────────────────────────────

std::string htmlEscape(const std::string& s) {
    std::string r;
    for (char c : s) {
        switch (c) {
            case '&':  r += "&amp;";  break;
            case '<':  r += "&lt;";   break;
            case '>':  r += "&gt;";   break;
            case '"':  r += "&quot;"; break;
            case '\'': r += "&#39;";  break;
            default:   r += c;
        }
    }
    return r;
}

// ─── Build full HTML page ───────────────────────────────────────────────────

std::string buildPage(const std::string& input, const std::string& selected) {

    // Compute results only when there is input
    std::vector<std::pair<std::string,std::string>> results;
    if (!input.empty()) {
        results = {
            {"UPPERCASE",       toUpperCase(input)},
            {"lowercase",       toLowerCase(input)},
            {"Title Case",      toTitleCase(input)},
            {"Sentence case",   toSentenceCase(input)},
            {"camelCase",       toCamelCase(input)},
            {"PascalCase",      toPascalCase(input)},
            {"snake_case",      toSnakeCase(input)},
            {"kebab-case",      toKebabCase(input)},
            {"AlTeRnAtInG",     toAlternatingCase(input)},
            {"iNVERSE cASE",    toInverseCase(input)},
        };
    }

    // Result cards
    std::string cards;
    for (const auto& r : results) {
        bool active = (r.first == selected);
        cards +=
            "<div class='card" + std::string(active ? " highlight" : "") + "'>"
            "<span class='label'>" + r.first + "</span>"
            "<form method='POST' action='/select' style='display:inline'>"
            "<input type='hidden' name='text' value='" + htmlEscape(input) + "'>"
            "<input type='hidden' name='case' value='" + htmlEscape(r.first) + "'>"
            "<button class='copy-btn' type='submit'>Use</button>"
            "</form>"
            "<div class='result'>" + htmlEscape(r.second) + "</div>"
            "</div>";
    }

    // Selected output box
    std::string selectedHtml;
    if (!selected.empty()) {
        // find the value for selected case
        std::string val;
        for (const auto& r : results)
            if (r.first == selected) { val = r.second; break; }
        selectedHtml =
            "<div class='selected-box'>"
            "<div class='sel-label'>&#10003; Active: <strong>" + htmlEscape(selected) + "</strong></div>"
            "<div class='sel-value'>" + htmlEscape(val) + "</div>"
            "</div>";
    }

    // Build HTML using string concatenation (avoids raw-string delimiter issues)
    std::string html;
    html += "<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "<meta charset=\"UTF-8\">\n"
            "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
            "<title>Piscok Market - Text Case Generator</title>\n"
            "<style>\n"
            "  *{box-sizing:border-box;margin:0;padding:0}\n"
            "  body{font-family:'Segoe UI',system-ui,sans-serif;background:#f0f4f8;color:#1f2328;min-height:100vh}\n"
            "  nav{background:#1a1a2e;color:#fff;padding:0 2rem;height:56px;display:flex;align-items:center;justify-content:space-between;box-shadow:0 2px 8px rgba(0,0,0,.35)}\n"
            "  nav .brand{font-size:1.3rem;font-weight:700;letter-spacing:.5px;color:#e2b96f}\n"
            "  nav .tagline{font-size:.8rem;color:#aab4be}\n"
            "  .hero{background:#16213e;color:#fff;padding:2.5rem 2rem 2rem}\n"
            "  .hero h1{font-size:1.8rem;margin-bottom:.4rem}\n"
            "  .hero p{color:#aab4be;font-size:.95rem}\n"
            "  .container{max-width:860px;margin:0 auto;padding:1.5rem 1rem}\n"
            "  .input-section{background:#fff;border-radius:10px;padding:1.5rem;box-shadow:0 1px 4px rgba(0,0,0,.08);margin-bottom:1.5rem}\n"
            "  .input-section h2{font-size:1rem;color:#57606a;margin-bottom:.8rem;font-weight:600;text-transform:uppercase;letter-spacing:.6px}\n"
            "  textarea{width:100%;border:1.5px solid #d0d7de;border-radius:8px;padding:.75rem 1rem;font-size:1rem;resize:vertical;min-height:90px;font-family:inherit}\n"
            "  textarea:focus{outline:none;border-color:#3b82d4}\n"
            "  .btn-row{display:flex;gap:.75rem;margin-top:.9rem;flex-wrap:wrap}\n"
            "  .btn{padding:.55rem 1.3rem;border:none;border-radius:7px;cursor:pointer;font-size:.95rem;font-weight:600}\n"
            "  .btn-primary{background:#1a1a2e;color:#e2b96f}\n"
            "  .btn-clear{background:#e5e7eb;color:#374151}\n"
            "  .selected-box{background:#fff9ec;border:1.5px solid #e2b96f;border-radius:10px;padding:1rem 1.3rem;margin-bottom:1.5rem}\n"
            "  .sel-label{font-size:.85rem;color:#57606a;margin-bottom:.4rem}\n"
            "  .sel-value{font-size:1.1rem;font-weight:600;word-break:break-all;color:#1a1a2e}\n"
            "  .grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(260px,1fr));gap:1rem}\n"
            "  .card{background:#fff;border-radius:10px;padding:1rem 1.2rem;box-shadow:0 1px 4px rgba(0,0,0,.07);border:1.5px solid transparent}\n"
            "  .card:hover{border-color:#3b82d4}\n"
            "  .card.highlight{border-color:#e2b96f;background:#fffdf5}\n"
            "  .label{font-size:.78rem;font-weight:700;color:#57606a;text-transform:uppercase;letter-spacing:.6px}\n"
            "  .result{font-size:1.02rem;font-weight:500;margin-top:.4rem;word-break:break-all;color:#1f2328}\n"
            "  .copy-btn{float:right;background:#1a1a2e;color:#e2b96f;border:none;border-radius:5px;padding:.2rem .6rem;font-size:.78rem;cursor:pointer}\n"
            "  .empty{text-align:center;padding:3rem;color:#57606a;font-size:.95rem}\n"
            "  footer{text-align:center;padding:1.5rem;color:#57606a;font-size:.8rem;border-top:1px solid #e5e7eb;margin-top:1rem}\n"
            "</style>\n"
            "</head>\n"
            "<body>\n"
            "<nav>\n"
            "  <div><div class=\"brand\">Piscok Market</div>"
            "<div class=\"tagline\">Text Case Generator</div></div>\n"
            "  <div style=\"font-size:.8rem;color:#aab4be\">v1.0 | localhost:8080</div>\n"
            "</nav>\n"
            "<div class=\"hero\">\n"
            "  <h1>Text Case Generator</h1>\n"
            "  <p>Enter any text below and instantly convert it to 10 different case styles.</p>\n"
            "</div>\n"
            "<div class=\"container\">\n"
            "  <div class=\"input-section\">\n"
            "    <h2>Input Text</h2>\n"
            "    <form method=\"POST\" action=\"/convert\">\n"
            "      <textarea name=\"text\" placeholder=\"Type or paste your text here...\">"
          + htmlEscape(input)
          + "</textarea>\n"
            "      <div class=\"btn-row\">\n"
            "        <button class=\"btn btn-primary\" type=\"submit\">Convert</button>\n"
            "      </div>\n"
            "    </form>\n"
            "    <form method=\"POST\" action=\"/clear\" style=\"margin-top:.5rem\">\n"
            "      <button class=\"btn btn-clear\" type=\"submit\">Clear</button>\n"
            "    </form>\n"
            "  </div>\n";

    html += selectedHtml;

    if (results.empty()) {
        html += "<div class=\"empty\">Enter some text above and click Convert to see all case styles.</div>\n";
    } else {
        html += "<div class=\"grid\">" + cards + "</div>\n";
    }

    html += "</div>\n"
            "<footer>Made with love for <strong>Piscok Market</strong> &mdash; Text Case Generator &copy; 2024</footer>\n"
            "</body>\n"
            "</html>\n";

    return html;
}

// ─── Send HTTP response ──────────────────────────────────────────────────────

void sendResponse(SOCKET client, int status, const std::string& body,
                  const std::string& ct = "text/html; charset=utf-8") {
    std::string statusText = (status == 200) ? "OK" :
                             (status == 302) ? "Found" : "Not Found";
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status << " " << statusText << "\r\n"
        << "Content-Type: " << ct << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    std::string resp = oss.str();
    send(client, resp.c_str(), (int)resp.size(), 0);
}

void sendRedirect(SOCKET client, const std::string& location) {
    std::string body = "<html><body>Redirecting...</body></html>";
    std::ostringstream oss;
    oss << "HTTP/1.1 302 Found\r\n"
        << "Location: " << location << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    std::string resp = oss.str();
    send(client, resp.c_str(), (int)resp.size(), 0);
}

// ─── Request parsing ─────────────────────────────────────────────────────────

struct Request {
    std::string method;
    std::string path;
    std::string body;
};

Request parseRequest(const std::string& raw) {
    Request req;
    std::istringstream ss(raw);
    ss >> req.method >> req.path;
    // find blank line separating headers from body
    auto pos = raw.find("\r\n\r\n");
    if (pos != std::string::npos)
        req.body = raw.substr(pos + 4);
    return req;
}

// ─── Main ────────────────────────────────────────────────────────────────────

int main() {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    const int PORT = 8080;
    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        std::cerr << "socket() failed\n";
        return 1;
    }

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "bind() failed — is port " << PORT << " already in use?\n";
        return 1;
    }
    listen(server, 5);

    std::cout << "==============================================\n";
    std::cout << "  Piscok Market — Text Case Generator\n";
    std::cout << "  Running at http://localhost:" << PORT << "\n";
    std::cout << "  Press Ctrl+C to stop.\n";
    std::cout << "==============================================\n";

    // Session state (single-user, in-memory)
    std::string currentText;
    std::string currentSelected;

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET client = accept(server, (sockaddr*)&clientAddr, &clientLen);
        if (client == INVALID_SOCKET) continue;

        // Read request
        char buf[8192] = {};
        recv(client, buf, sizeof(buf) - 1, 0);
        std::string raw(buf);
        Request req = parseRequest(raw);

        if (req.method == "GET" && req.path == "/") {
            sendResponse(client, 200, buildPage(currentText, currentSelected));

        } else if (req.method == "POST" && req.path == "/convert") {
            auto params = parseForm(req.body);
            currentText     = params.count("text") ? params["text"] : "";
            currentSelected = "";
            sendResponse(client, 200, buildPage(currentText, currentSelected));

        } else if (req.method == "POST" && req.path == "/select") {
            auto params = parseForm(req.body);
            currentText     = params.count("text") ? params["text"] : currentText;
            currentSelected = params.count("case") ? params["case"] : "";
            sendResponse(client, 200, buildPage(currentText, currentSelected));

        } else if (req.method == "POST" && req.path == "/clear") {
            currentText.clear();
            currentSelected.clear();
            sendResponse(client, 200, buildPage(currentText, currentSelected));

        } else {
            sendRedirect(client, "/");
        }

        closesocket(client);
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
