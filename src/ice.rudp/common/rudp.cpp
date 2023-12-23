#include "rudp.h"

#pragma region SCHEDULER

bool scheduler::empty()
{
	return head == nullptr ? true : false;
}

scheduler::element* scheduler::add(std::function<void()> new_event, int time_in_milliseconds)
{
	element* new_element = new element;

	new_element->event = new_event;
	new_element->event_time = std::chrono::system_clock::now() + std::chrono::milliseconds(time_in_milliseconds);

	if (head == nullptr)
	{
		head = new_element;

		return new_element;
	}

	if (new_element->event_time < head->event_time)
	{
		new_element->previous = head;

		head->next = new_element;
		head = new_element;

		return new_element;
	}

	element* window_head = nullptr;
	element* window_low = head;

	while (window_low != nullptr && new_element->event_time > window_low->event_time)
	{
		window_head = window_low;
		window_low = window_low->previous;
	}

	window_head == nullptr ? nullptr : (window_head->previous = new_element);
	if (window_low != nullptr) window_low->next = new_element;

	new_element->next = window_head;
	new_element->previous = window_low;

	return new_element;
}

void scheduler::remove(element*& obj)
{
	if (obj == nullptr) return;

	if (obj->next != nullptr) obj->next->previous = obj->previous;
	if (obj->previous != nullptr) obj->previous->next = obj->next;

	if (obj == head) head = head->previous;

	delete obj;

	obj = nullptr;
}

void scheduler::execute()
{
	if (head == nullptr) return;

	if (head->event_time > std::chrono::system_clock::now()) return;

	element* temp = head;

	head = head->previous;
	temp->event();

	delete temp;
}

void scheduler::clear()
{
	while (head != nullptr) {
		element* temp = head;
		head = head->previous;
		delete temp;
	}

	head = nullptr;
}

#pragma endregion

#pragma region STOPWATCH

void stopwatch::start()
{
	elapsed_milliseconds = 0;
	start_time = std::chrono::system_clock::now();
}

void stopwatch::stop()
{
	elapsed_milliseconds = 0;
	elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();
}

unsigned long long stopwatch::get_elapsed_milliseconds()
{
	return elapsed_milliseconds;
}

#pragma endregion