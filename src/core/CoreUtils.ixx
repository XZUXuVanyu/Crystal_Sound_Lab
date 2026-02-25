module;
#include <iostream>
#include <format>
#include <filesystem>
#include <string>
#include <string_view>
#include <source_location>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
export module CrystalCore.CoreUtils;

#ifdef _DEBUG
constexpr bool IS_DEBUG = true;
#else
constexpr bool IS_DEBUG = false;
#endif

struct LogEntry
{
    std::string message;
    std::string fileName;
    uint32_t line;
};

class Logger
{
private:
    std::queue<LogEntry> queue;
    std::mutex mutex;
    std::condition_variable cv;
    std::thread watchThread;
    std::atomic<bool> exit{ false };

    Logger()
    {
        if constexpr (IS_DEBUG) watchThread = std::thread(&Logger::processLogs, this);
    }

    void processLogs() {
        while (!exit or !queue.empty())
        {
            std::unique_lock lock(mutex);
            cv.wait(lock, [this] { return !queue.empty() or exit; });

            while (!queue.empty()) 
            {
                auto entry = std::move(queue.front());
                queue.pop();
                lock.unlock();

                std::cout << std::format("[DEBUG] \"{}\": line<{}> -> {}",
                    entry.fileName, entry.line, entry.message) << std::endl;

                lock.lock();
            }
        }
    }

public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    ~Logger() {
        exit = true;
        cv.notify_all();
        if (watchThread.joinable()) watchThread.join();
    }

    void push(LogEntry&& entry) {
        std::lock_guard lock(mutex);
        queue.push(std::move(entry));
        cv.notify_one();
    }
};

template<typename... Args>
struct FmtLoc {
    std::format_string<Args...> fmt;
    std::source_location loc;

    template<typename T>
    consteval FmtLoc(const T& s, std::source_location l = std::source_location::current())
        : fmt(s), loc(l) {
    }
};

export template<typename... Args>
void DEBUG_LOG(std::type_identity_t<FmtLoc<Args...>> fmt_obj, Args&&... args)
{
    if constexpr (IS_DEBUG)
    {
        std::string msg = std::format(fmt_obj.fmt, std::forward<Args>(args)...);
        std::filesystem::path path = fmt_obj.loc.file_name();

        Logger::getInstance().push({ std::move(msg), path.filename().string(), fmt_obj.loc.line() });
    }
}