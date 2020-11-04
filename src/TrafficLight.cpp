#include <iostream>
#include <random>
#include <thread>
#include <mutex>
#include "TrafficLight.h"

std::mutex mtx;
std::condition_variable _condition;
/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lock(mtx);
    _condition.wait(lock, [this] {return !_queue.empty(); });
    T obj = std::move(_queue.back());
    _queue.clear(); // clear queue
    std::cout << obj << "received" << std::endl; //message 
    return obj;
}

template <typename T>
void MessageQueue<T>::send(T&& msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(mtx); 
    std::cout << msg << "has been added to queue" << std::endl;
    _queue.push_back(std::move(msg)); //push current phase in queue
    _condition.notify_one(); //send one-time notification
}


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        auto message = _queue.receive();
        if (message == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the 
    // public method „simulate“ is called. To do this, use the thread queue in the base class. 
    TrafficObject::threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

// generate random generation between 4 and 6 seconds using <random>
std::random_device random;    
std::mt19937 eng(random());
std::uniform_int_distribution<> distr(4000, 6000); // 4s - 6s 

// Initalize variables for cycle
int cycle_time = distr(eng);     
// Create stop watch    
auto lastCycle = std::chrono::system_clock::now();
while(true)
{   
    std::this_thread::sleep_for(std::chrono::milliseconds(1));  // Sleep between iterations      
    long timeSinceLastCycle = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastCycle).count();        
    if (timeSinceLastCycle >= cycle_time)
    {     // toggle through phases     
        if(_currentPhase == TrafficLightPhase::green)
        {               
            _currentPhase = TrafficLightPhase::red;    
        }            
        else
        {  
            _currentPhase = TrafficLightPhase::green;
        }  
           _queue.send(std::move(_currentPhase));            
    // Reset stop watch            
    lastCycle = std::chrono::system_clock::now();            
    // Again, choose the cycle duration for the next cycle randomly           
    cycle_time = distr(eng);
    }
}
}

