#include <iostream>
#include <map>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
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

// Boost.Asio tutorials
//############################################# BEGIN #########################################################
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

class printer
{
public:
    printer(asio_io_context& io_context, boost::asio::chrono::seconds secs = boost::asio::chrono::seconds(1)) :
        m_steady_timer(io_context, secs),
        m_count(0)
    {
        m_steady_timer.async_wait(boost::bind(&printer::print, this));
    }

    ~printer()
    {
        std::cout << "Final count down: " << m_count << '\n';
    }

    void print()
    {
        if (m_count < 5)
        {
            std::cout << "count: " << m_count << '\n';
            ++m_count;
            m_steady_timer.expires_at(m_steady_timer.expiry() + boost::asio::chrono::seconds(1));
            m_steady_timer.async_wait(boost::bind(&printer::print, this));
        }
    }
private:
    asio_steady_timer m_steady_timer;
    int m_count;
};

void async_timer_like_member_function_example(asio_io_context& io_context, bool invoke = false)
{
    if (!invoke) return;

    printer printer_v{io_context};
    io_context.run();
}


class parallel_printer
{
public:
    parallel_printer(asio_io_context& io_context
                     , boost::asio::chrono::seconds sec_1 = boost::asio::chrono::seconds(1)
                     , boost::asio::chrono::seconds sec_2 = boost::asio::chrono::seconds(1)) :
        m_strand(boost::asio::make_strand(io_context)),
        m_steady_timer_1(io_context, sec_1),
        m_steady_timer_2(io_context, sec_2),
        m_count(0)
    {
        m_steady_timer_1.async_wait(boost::asio::bind_executor(m_strand, std::bind(&parallel_printer::print_1, this)));
        m_steady_timer_2.async_wait(boost::asio::bind_executor(m_strand, std::bind(&parallel_printer::print_2, this)));

    }

    ~parallel_printer()
    {
        std::cout << "Final count down: " << m_count << '\n';
    }

    void print_1()
    {
        if (m_count < 10)
        {
            std::cout << "Timer 1 has set m_count to: " << m_count << '\n';
            ++m_count;

            m_steady_timer_1.expires_at(m_steady_timer_1.expiry() + boost::asio::chrono::seconds(1));
            m_steady_timer_1.async_wait(boost::asio::bind_executor(m_strand, boost::bind(&parallel_printer::print_1, this)));
        }
    }
    void print_2()
    {
        if (m_count < 10)
        {
            std::cout << "Timer 2 has set m_count to: " << m_count << '\n';
            ++m_count;

            m_steady_timer_2.expires_at(m_steady_timer_2.expiry() + boost::asio::chrono::seconds(1));
            m_steady_timer_2.async_wait(boost::asio::bind_executor(m_strand, boost::bind(&parallel_printer::print_2, this)));
        }
    }
private:
    boost::asio::strand<boost::asio::io_context::executor_type> m_strand;
    asio_steady_timer m_steady_timer_1;
    asio_steady_timer m_steady_timer_2;
    int m_count;
};

void async_parallel_member_function_timer_example(asio_io_context& io_context, bool invoke = false)
{
    if (!invoke) return;

    parallel_printer pp{io_context};

    boost::thread thread(boost::bind(&asio_io_context::run, &io_context));

    io_context.run();

    thread.join();
}

// Boost.Asio tutorials
//############################################## END ##########################################################

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

        auto invoke_async_timer_with_bind_function{false};
        async_timer_with_bind_func_example(io_context, invoke_async_timer_with_bind_function);

        auto invoke_async_timer_like_member_function{false};
        async_timer_like_member_function_example(io_context, invoke_async_timer_like_member_function);

        auto invoke_async_parallel_member_function_timer{true};
        async_parallel_member_function_timer_example(io_context, invoke_async_parallel_member_function_timer);
        return 0;
    }
    catch (std::exception& e)
    {
        std::cerr << "ERROR:" << e.what() << '\n';
    }

    return 0;
}
