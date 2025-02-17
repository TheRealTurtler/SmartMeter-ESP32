#include <WebServer.h>

class HttpAPI
{
public:
	HttpAPI(uint16_t port = 80);
	~HttpAPI() = default;

	void init();
	void start();
	void stop();
	void update();

private:
	WebServer m_server;

	void getHome();
	void getPing();
	void getStatus();
	void getRaw();
};
