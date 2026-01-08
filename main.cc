#include <drogon/drogon.h>
#include "Global.h"
#include "clsSemester.h"

using namespace drogon;
using namespace std;

void addCorsHeaders(const HttpResponsePtr& resp) {
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With, ngrok-skip-browser-warning");
}

int main() {

    clsSemester sem = clsSemester::GetCurrent();
    if (sem.Name != "") {
        CurrentSemester = sem.Name;
        cout << ">>> Active Semester loaded from DB: " << CurrentSemester << endl;
    }

    app().registerHandler(
        "/{path:.*}",
        [](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb) {
            if (req->method() == Options) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k200OK);
                addCorsHeaders(resp);
                cb(resp);
                return;
            }
            auto resp = HttpResponse::newNotFoundResponse();
            addCorsHeaders(resp);
            cb(resp);
        },
        { Options }
    );

    app().registerPostHandlingAdvice([](const HttpRequestPtr& req, const HttpResponsePtr& resp) {
        addCorsHeaders(resp);
        });

    std::cout << ">>> Server Started.\n";
    std::cout << ">>> Listening on port 8088...\n";

    app().addListener("0.0.0.0", 8088).run();

    return 0;
}