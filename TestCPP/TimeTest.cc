// time_point constructors
#include <iostream>
#include <chrono>
#include <ctime>
 
void CaclAlgorithm(){


// 给算法计时
    using namespace std::chrono;
 
  steady_clock::time_point t1 = steady_clock::now();
 
  std::cout << "printing out 1000 stars...\n";
  for (int i=0; i<1000; ++i) std::cout << "*";
  std::cout << std::endl;
 
  steady_clock::time_point t2 = steady_clock::now();
 
  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
 
  std::cout << "It took me " << time_span.count() << " seconds.";
  std::cout << std::endl;

}

int deterDur()
{
    using namespace std::chrono;
    //ratio<60*60*24> means one day
    //ratio<1,1000> means mills
    //mills_type: now, end
    //end.time_since_epoch().count() - now.time_since_epoch().count() means runtime
 
    typedef duration<int,std::ratio<60*60*24>> days_type;
 
    time_point<system_clock,days_type> today = time_point_cast<days_type>(system_clock::now());
 
    std::cout << today.time_since_epoch().count() << " days since epoch" << std::endl;
}

void CaclNextDay()
{

 using std::chrono::system_clock;
 
  std::chrono::duration<int,std::ratio<60*60*24> > one_day (1);
 
  system_clock::time_point today = system_clock::now();
  system_clock::time_point tomorrow = today + one_day;
 
  std::time_t tt;
 
  tt = system_clock::to_time_t ( today );
  std::cout << "today is: " << ctime(&tt);
 
  tt = system_clock::to_time_t ( tomorrow );
  std::cout << "tomorrow will be: " << ctime(&tt);

}

std::time_t getTimeStamp()
{
    std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto tmp=std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    //std::time_t timestamp = tmp.count();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(tp);
    return timestamp;
}

int main ()
{
    /*
  using namespace std::chrono;
 
  system_clock::time_point tp_epoch;    // epoch value
 
  time_point <system_clock,duration<int>> tp_seconds (duration<int>(1));
 
  system_clock::time_point tp (tp_seconds);
 
  std::cout << "1 second since system_clock epoch = ";
  std::cout << tp.time_since_epoch().count();
  std::cout << " system_clock periods." << std::endl;
  */
 
  // display time_point:
  //std::time_t tt = system_clock::to_time_t(tp);
  std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  auto tp = getTimeStamp();
  std::cout << "time_point tp is: " << ctime(&tt);
  std::cout << "time_point tp is: " << ctime(&tp);
  //std::cout << "time_point tp is: " << tt;
  deterDur();
  CaclNextDay();
  return 0;
}