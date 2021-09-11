#include <iostream>
#include <random>
#include <thread>
#include <mutex>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  	std::unique_lock<std::mutex> u_lock(_mtx);
  	_cndx.wait(u_lock, [this]{ return !_queue.empty(); });
    T message = std::move(_queue.back());
  	_queue.pop_back();
  	return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  	std::lock_guard<std::mutex> lg(_mtx);
  	_queue.push_front(std::move(msg));
    _cndx.notify_one();
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
  	while(true){
    	TrafficLightPhase msg = _messages.receive();
      	if (msg == TrafficLightPhase::green){
        	return;
        } 
      	std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
  threads.emplace_back( std::thread( &TrafficLight::cycleThroughPhases, this) );
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator( seed );
  std::uniform_int_distribution<int> distribution(4000,6000);
  auto target = distribution( generator );

  auto start_time = std::chrono::steady_clock::now();
  while ( true ) {
    auto cur_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds> (cur_time - start_time).count();

    if ( elapsed >= target ) {
      _currentPhase = (_currentPhase == TrafficLightPhase::green) ?
                      TrafficLightPhase::red : TrafficLightPhase::green;

      _messages.send(std::move(TrafficLightPhase(_currentPhase)));
      start_time = std::chrono::steady_clock::now();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

