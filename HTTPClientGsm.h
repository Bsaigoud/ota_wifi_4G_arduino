/**
 * @file HTTPClientGsm.h
 * @author S GOPIKRISHNA
 * @brief
 * @version 0.1
 * @date 2024-10-09
 *
 * @copyright Copyright (c) 2024
 * @patanet EVRE
 */

#ifndef HTTPClientGsm_H_
#define HTTPClientGsm_H_

// #ifndef HTTPCLIENT_1_1_COMPATIBLE
// #define HTTPCLIENT_1_1_COMPATIBLE
// #endif

#include <memory>
#include <Arduino.h>
#define TINY_GSM_MODEM_SIM7600
#include <TinyGsmClient.h>

/// Cookie jar support
#include <vector>

#define DEBUG_ENABLE (0)
#define HTTPCLIENT_DEFAULT_TCP_TIMEOUT_GSM (5000)

/// HTTP client errors
#define HTTPC_ERROR_CONNECTION_REFUSED_GSM (-1)
#define HTTPC_ERROR_SEND_HEADER_FAILED_GSM (-2)
#define HTTPC_ERROR_SEND_PAYLOAD_FAILED_GSM (-3)
#define HTTPC_ERROR_NOT_CONNECTED_GSM (-4)
#define HTTPC_ERROR_CONNECTION_LOST_GSM (-5)
#define HTTPC_ERROR_NO_STREAM_GSM (-6)
#define HTTPC_ERROR_NO_HTTP_SERVER_GSM (-7)
#define HTTPC_ERROR_TOO_LESS_RAM_GSM (-8)
#define HTTPC_ERROR_ENCODING_GSM (-9)
#define HTTPC_ERROR_STREAM_WRITE_GSM (-10)
#define HTTPC_ERROR_READ_TIMEOUT_GSM (-11)

/// size for the stream handling
#define HTTP_TCP_BUFFER_SIZE_GSM (1460)

/// HTTP codes see RFC7231
typedef enum
{
    HTTP_CODE_CONTINUE_GSM = 100,
    HTTP_CODE_SWITCHING_PROTOCOLS_GSM = 101,
    HTTP_CODE_PROCESSING_GSM = 102,
    HTTP_CODE_OK_GSM = 200,
    HTTP_CODE_CREATED_GSM = 201,
    HTTP_CODE_ACCEPTED_GSM = 202,
    HTTP_CODE_NON_AUTHORITATIVE_INFORMATION_GSM = 203,
    HTTP_CODE_NO_CONTENT_GSM = 204,
    HTTP_CODE_RESET_CONTENT_GSM = 205,
    HTTP_CODE_PARTIAL_CONTENT_GSM = 206,
    HTTP_CODE_MULTI_STATUS_GSM = 207,
    HTTP_CODE_ALREADY_REPORTED_GSM = 208,
    HTTP_CODE_IM_USED_GSM = 226,
    HTTP_CODE_MULTIPLE_CHOICES_GSM = 300,
    HTTP_CODE_MOVED_PERMANENTLY_GSM = 301,
    HTTP_CODE_FOUND_GSM = 302,
    HTTP_CODE_SEE_OTHER_GSM = 303,
    HTTP_CODE_NOT_MODIFIED_GSM = 304,
    HTTP_CODE_USE_PROXY_GSM = 305,
    HTTP_CODE_TEMPORARY_REDIRECT_GSM = 307,
    HTTP_CODE_PERMANENT_REDIRECT_GSM = 308,
    HTTP_CODE_BAD_REQUEST_GSM = 400,
    HTTP_CODE_UNAUTHORIZED_GSM = 401,
    HTTP_CODE_PAYMENT_REQUIRED_GSM = 402,
    HTTP_CODE_FORBIDDEN_GSM = 403,
    HTTP_CODE_NOT_FOUND_GSM = 404,
    HTTP_CODE_METHOD_NOT_ALLOWED_GSM = 405,
    HTTP_CODE_NOT_ACCEPTABLE_GSM = 406,
    HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED_GSM = 407,
    HTTP_CODE_REQUEST_TIMEOUT_GSM = 408,
    HTTP_CODE_CONFLICT_GSM = 409,
    HTTP_CODE_GONE_GSM = 410,
    HTTP_CODE_LENGTH_REQUIRED_GSM = 411,
    HTTP_CODE_PRECONDITION_FAILED_GSM = 412,
    HTTP_CODE_PAYLOAD_TOO_LARGE_GSM = 413,
    HTTP_CODE_URI_TOO_LONG_GSM = 414,
    HTTP_CODE_UNSUPPORTED_MEDIA_TYPE_GSM = 415,
    HTTP_CODE_RANGE_NOT_SATISFIABLE_GSM = 416,
    HTTP_CODE_EXPECTATION_FAILED_GSM = 417,
    HTTP_CODE_MISDIRECTED_REQUEST_GSM = 421,
    HTTP_CODE_UNPROCESSABLE_ENTITY_GSM = 422,
    HTTP_CODE_LOCKED_GSM = 423,
    HTTP_CODE_FAILED_DEPENDENCY_GSM = 424,
    HTTP_CODE_UPGRADE_REQUIRED_GSM = 426,
    HTTP_CODE_PRECONDITION_REQUIRED_GSM = 428,
    HTTP_CODE_TOO_MANY_REQUESTS_GSM = 429,
    HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE_GSM = 431,
    HTTP_CODE_INTERNAL_SERVER_ERROR_GSM = 500,
    HTTP_CODE_NOT_IMPLEMENTED_GSM = 501,
    HTTP_CODE_BAD_GATEWAY_GSM = 502,
    HTTP_CODE_SERVICE_UNAVAILABLE_GSM = 503,
    HTTP_CODE_GATEWAY_TIMEOUT_GSM = 504,
    HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED_GSM = 505,
    HTTP_CODE_VARIANT_ALSO_NEGOTIATES_GSM = 506,
    HTTP_CODE_INSUFFICIENT_STORAGE_GSM = 507,
    HTTP_CODE_LOOP_DETECTED_GSM = 508,
    HTTP_CODE_NOT_EXTENDED_GSM = 510,
    HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED_GSM = 511
} t_http_codes_gsm;

typedef enum
{
    HTTPC_TE_IDENTITY_GSM,
    HTTPC_TE_CHUNKED_GSM
} transferEncoding_t_gsm;

/**
 * redirection follow mode.
 * + `HTTPC_DISABLE_FOLLOW_REDIRECTS` - no redirection will be followed.
 * + `HTTPC_STRICT_FOLLOW_REDIRECTS` - strict RFC2616, only requests using
 *      GET or HEAD methods will be redirected (using the same method),
 *      since the RFC requires end-user confirmation in other cases.
 * + `HTTPC_FORCE_FOLLOW_REDIRECTS` - all redirections will be followed,
 *      regardless of a used method. New request will use the same method,
 *      and they will include the same body data and the same headers.
 *      In the sense of the RFC, it's just like every redirection is confirmed.
 */
typedef enum
{
    HTTPC_DISABLE_FOLLOW_REDIRECTS_GSM,
    HTTPC_STRICT_FOLLOW_REDIRECTS_GSM,
    HTTPC_FORCE_FOLLOW_REDIRECTS_GSM
} followRedirects_t_gsm;

#ifdef HTTPCLIENT_1_1_COMPATIBLE
class TransportTraits;
typedef std::unique_ptr<TransportTraits> TransportTraitsPtr;
#endif

class TransportTraits;
typedef std::unique_ptr<TransportTraits> TransportTraitsPtr;

// cookie jar support
typedef struct
{
    String host; // host which tries to set the cookie
    time_t date; // timestamp of the response that set the cookie
    String name;
    String value;
    String domain;
    String path = "";
    struct
    {
        time_t date = 0;
        bool valid = false;
    } expires;
    struct
    {
        time_t duration = 0;
        bool valid = false;
    } max_age;
    bool http_only = false;
    bool secure = false;
} Cookie_gsm;
typedef std::vector<Cookie_gsm> CookieJar_gsm;

class HTTPClient_gsm
{
public:
    HTTPClient_gsm();
    ~HTTPClient_gsm();

    /*
     * Since both begin() functions take a reference to client as a parameter, you need to
     * ensure the client object lives the entire time of the HTTPClient
     */
    bool begin(TinyGsmClient &client, String url);
    bool begin(TinyGsmClient &client, String host, uint16_t port, String uri = "/", bool https = false);

#ifdef HTTPCLIENT_1_1_COMPATIBLE
    bool begin(String url);
    bool begin(String url, const char *CAcert);
    bool begin(String host, uint16_t port, String uri = "/");
    bool begin(String host, uint16_t port, String uri, const char *CAcert);
    bool begin(String host, uint16_t port, String uri, const char *CAcert, const char *cli_cert, const char *cli_key);
#else
    bool begin(String url);
    bool begin(String url, const char *CAcert);
#endif
    void end(void);

    bool connected(void);

    void setReuse(bool reuse); /// keep-alive
    void setUserAgent(const String &userAgent);
    void setAuthorization(const char *user, const char *password);
    void setAuthorization(const char *auth);
    void setAuthorizationType(const char *authType);
    void setConnectTimeout(int32_t connectTimeout);
    void setTimeout(uint16_t timeout);

    // Redirections
    void setFollowRedirects(followRedirects_t_gsm follow);
    void setRedirectLimit(uint16_t limit); // max redirects to follow for a single request

    bool setURL(const String &url);
    void useHTTP10(bool usehttp10 = true);

    /// request handling
    int GET();
    int PATCH(uint8_t *payload, size_t size);
    int PATCH(String payload);
    int POST(uint8_t *payload, size_t size);
    int POST(String payload);
    int PUT(uint8_t *payload, size_t size);
    int PUT(String payload);
    int sendRequest(const char *type, String payload);
    int sendRequest(const char *type, uint8_t *payload = NULL, size_t size = 0);
    int sendRequest(const char *type, Stream *stream, size_t size = 0);

    void addHeader(const String &name, const String &value, bool first = false, bool replace = true);

    /// Response handling
    void collectHeaders(const char *headerKeys[], const size_t headerKeysCount);
    String header(const char *name);  // get request header value by name
    String header(size_t i);          // get request header value by number
    String headerName(size_t i);      // get request header name by number
    int headers();                    // get header count
    bool hasHeader(const char *name); // check if header exists

    int getSize(void);
    const String &getLocation(void);

    TinyGsmClient &getStream(void);
    TinyGsmClient *getStreamPtr(void);
    int writeToStream(Stream *stream);
    String getString(void);

    static String errorToString(int error);

    /// Cookie jar support
    void setCookieJar(CookieJar_gsm *cookieJar);
    void resetCookieJar();
    void clearAllCookies();

protected:
    struct RequestArgument
    {
        String key;
        String value;
    };

    bool beginInternal(String url, const char *expectedProtocol);
    void disconnect(bool preserveClient = false);
    void clear();
    int returnError(int error);
    bool connect(void);
    bool sendHeader(const char *type);
    int handleHeaderResponse();
    int writeToStreamDataBlock(Stream *stream, int len);

    /// Cookie jar support
    void setCookie(String date, String headerValue);
    bool generateCookieString(String *cookieString);

#ifdef HTTPCLIENT_1_1_COMPATIBLE
    TransportTraitsPtr _transportTraits;
    std::unique_ptr<TinyGsmClient> _tcpDeprecated;
#else
    TransportTraitsPtr _transportTraits;
    std::unique_ptr<TinyGsmClient> _tcpDeprecated;
    TinyGsmClient *_client = nullptr;
#endif

    /// request handling
    String _host;
    uint16_t _port = 0;
    int32_t _connectTimeout = HTTPCLIENT_DEFAULT_TCP_TIMEOUT_GSM;
    bool _reuse = true;
    uint16_t _tcpTimeout = HTTPCLIENT_DEFAULT_TCP_TIMEOUT_GSM;
    bool _useHTTP10 = false;
    bool _secure = false;

    String _uri;
    String _protocol;
    String _headers;
    String _userAgent = "ESP32HTTPClient";
    String _base64Authorization;
    String _authorizationType = "Basic";

    /// Response handling
    RequestArgument *_currentHeaders = nullptr;
    size_t _headerKeysCount = 0;

    int _returnCode = 0;
    int _size = -1;
    bool _canReuse = false;
    followRedirects_t_gsm _followRedirects = HTTPC_DISABLE_FOLLOW_REDIRECTS_GSM;
    uint16_t _redirectLimit = 10;
    String _location;
    transferEncoding_t_gsm _transferEncoding = HTTPC_TE_IDENTITY_GSM;

    /// Cookie jar support
    CookieJar_gsm *_cookieJar = nullptr;
};

#endif /* HTTPClient_H_ */
