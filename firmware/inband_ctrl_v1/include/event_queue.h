#ifndef _EVENT_QUEUE_H_
#define _EVENT_QUEUE_H_

//! Event message used for timing of events
typedef struct {
	//!The function we wish to run at the specified time
	void (*func)(void);
	//!The target time where we wish to event to occur
	unsigned int time_target;
	//!The event id, can be used to drop a certain type of messages
	unsigned char id;
} EVENT_MESSAGE;

//! Event list with size EVENT_LIST_SIZE
EVENT_MESSAGE event_list[EVENT_LIST_SIZE];

void event_queue_init(void);
char event_queue_add(EVENT_MESSAGE event);
EVENT_MESSAGE event_queue_get(void);
void event_queue_drop(void);
unsigned char event_queue_count(void);
void event_queue_dropall(void);
unsigned char event_in_queue(void);
void event_queue_wrap(unsigned int remove_val);
int event_queue_drop_id(unsigned char id);
unsigned char event_queue_check_id(unsigned char id);

#endif
