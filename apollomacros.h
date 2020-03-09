// Connection macros
#define APOLLO_URL "10.50.202.122"
#define APOLLO_PORT 3000
#define APOLLO_FINGERPRINT ""

// Defining general macros
#define MAX_CONFIG_TRIES 30
#define SSID_SIZE 32
#define PASSPHRASE_SIZE 32
#define DEVICEID_SIZE 32
#define APIKEY_SIZE 32
#define TOKEN_SIZE 512
#define IP_SIZE 16
#define FINGERPRINT_SIZE 256

// Ping interval
#define PING_INTERVAL 5

// Defining macros for Apollo states
#define WIFI_NOT_CONNECTED 0
#define WIFI_CONNECTED 1
#define DUPLEX_CONNECTED 2

// Indexes for functions in eventQueue
#define LOGOUT 0

// Indexes for handlers callbacks
#define ONCONNECTED 0
#define ONDISCONNECTED 1
#define ONMESSAGE 2