#include <iostream>
#include <map>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace std;
using namespace rapidjson;


void json_example (bool invoke = false)
{
    if (!invoke)
    {
        std::cout << "JSON example has been skiped\n";
        return;
    }

    std::cout << "JSON example:\n";
    const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document d;
    d.Parse(json);

    // 2. Modify it by DOM.
    Value& s = d["stars"];
    s.SetInt(s.GetInt() + 1);

    // 3. Stringify the DOM
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    // Output {"project":"rapidjson","stars":11}
    std::cout << buffer.GetString() << '\n';
}

using asio_io_context = boost::asio::io_context;
using asio_tcp_socket = boost::asio::ip::tcp::socket;
using asio_steady_timer = boost::asio::steady_timer;

void sync_timer_example(asio_io_context& io_context, bool invoke = false)
{
    if (!invoke) return;

    asio_steady_timer steady_timer{io_context, boost::asio::chrono::seconds(5)};

    steady_timer.wait();

    std::cout << "Hello, world! Sync timer just has been expired!\n";
}


void print(const boost::system::error_code& /*ec*/)
{
    std::cout << "Hello, world! Async timer just has been expired!\n";
}

void async_timer_example(asio_io_context& io_context, bool invoke = false)
{
    if (!invoke) return;

    asio_steady_timer steady_timer{io_context, boost::asio::chrono::seconds(5)};

    steady_timer.async_wait(&print);

    io_context.run();
}

void print(const boost::system::error_code& /*ec*/,
           asio_steady_timer* steady_time,
           int* count)
{
    if (*count < 5)
    {
        std::cout << "count: " << *count << '\n';
        ++(*count);
        steady_time->expires_at(steady_time->expiry() + boost::asio::chrono::seconds(1));
        steady_time->async_wait(boost::bind(print, boost::asio::placeholders::error, steady_time, count));
    }
}

void async_timer_with_bind_func_example(asio_io_context& io_context, bool invoke)
{
    if (!invoke) return;

    auto count{0};
    asio_steady_timer steady_timer{io_context, boost::asio::chrono::seconds(1)};

    steady_timer.async_wait(boost::bind(print, boost::asio::placeholders::error, &steady_timer, &count));

    io_context.run();
    std::cout << "Async timer with bind func exmaple finished!\n";
}

int main()
{
    try
    {
        auto invoke_json_example{false};
        json_example(invoke_json_example);

        asio_io_context io_context;

        auto invoke_sync_timer{false};
        sync_timer_example(io_context, invoke_sync_timer);

        auto invoke_async_timer{false};
        async_timer_example(io_context, invoke_async_timer);

        auto invoke_async_timer_with_bind_function{true};
        async_timer_with_bind_func_example(io_context, invoke_async_timer_with_bind_function);
        return 0;
    }
    catch (std::exception& e)
    {
        std::cerr << "ERROR:" << e.what() << '\n';
    }

    return 0;
}
