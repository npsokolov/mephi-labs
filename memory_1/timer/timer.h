#include <iostream>
#include <chrono>
#include <cmath>
#include <sstream>

class Timer {
public:
    Timer(std::chrono::duration<long long int> dur_): dur(dur_), x1(std::chrono::duration_cast<std::chrono::milliseconds>(dur).count()){
        StartTime = std::chrono::steady_clock::now();
    }
    bool Expired() const
    {
        auto NowTime=std::chrono::steady_clock::now();
        auto x = std::chrono::duration_cast<std::chrono::milliseconds>(NowTime - StartTime).count();
        return (x>x1);

    }
private:
    std::chrono::_V2::steady_clock::time_point StartTime;
    std::chrono::duration<long long int> dur;
    int64_t x1;
};

class TimeMeasurer {
public:
    TimeMeasurer(std::ostream &Ostream_): Ostream1(Ostream_) 
    {
        StartTime = std::chrono::steady_clock::now();
    }
    ~TimeMeasurer()
    {
        EndTime = std::chrono::steady_clock::now();
        int64_t x = std::chrono::duration_cast<std::chrono::milliseconds>(EndTime - StartTime).count()-5;
        Ostream1 << "Elapsed time: " << x << std::endl;
    }
private:
    std::chrono::_V2::steady_clock::time_point StartTime;
    std::chrono::_V2::steady_clock::time_point EndTime;
    std::ostream& Ostream1;
};