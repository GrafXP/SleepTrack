#include <pebble.h>
#include <math.h>

Window *window;
TextLayer *text_layer;
int sentFlag = 1;
int oldVal = 0;

uint32_t valArr[30];
uint32_t timeStampArr[30];
int valIndex = 0;

static bool send_data_to_phone(){
	while(!sentFlag){
		
	}
	
	DictionaryIterator *iter;

	int i = 0;
  app_message_outbox_begin(&iter);
	
	if (!iter) {
    // Error creating outbound message
    return false;
  }
	
	for(i=0;i<valIndex;++i){
		
		dict_write_int32(iter, timeStampArr[i], valArr[i]);
	}
  
  dict_write_end(iter);
  app_message_outbox_send();
	
	valIndex = 0;
	for(int i=0;i<25;++i){
		valArr[i] = 0;
	}

  return true;
}

static void data_handler(AccelData *data, uint32_t num_samples){
	
	int tmpValArr[25];
	
	for(int i = 1;i<25;i++){
		
		//int val1 = (abs(data[i-1].x) + abs(data[i-1].y) + abs(data[i-1].z));
		//int val2 = (abs(data[i].x) + abs(data[i].y) + abs(data[i].z));
		
		int val1 = (data[i-1].x + data[i-1].y + data[i-1].z);
		int val2 = (data[i].x + data[i].y + data[i].z);
		
		tmpValArr[i] = val2 - val1;
		
		int valx = abs(data[i].x - data[i-1].x);
		int valy = abs(data[i].y - data[i-1].y);
		int valz = abs(data[i].z - data[i-1].z);
		
		if(abs(tmpValArr[i]) > 50){
			
			//uint32_t result = abs((uint32_t)data[i].x);
			uint32_t result = 0;
			result = (uint32_t)(valx & 0b0000001111111111) << 20 | (uint32_t)(valy & 0b0000001111111111) << 10 | (uint32_t)(valz & 0b0000001111111111);
			
			valArr[valIndex] = result;
			//valArr[valIndex+1] = valx;
			//valArr[valIndex+2] = valy;
			//valArr[valIndex+3] = valz;
			
			unsigned long hi, lo;

			memcpy(&lo, &data[i].timestamp, sizeof lo);
			memcpy(&hi, (char *) &data[i].timestamp + sizeof lo, sizeof hi);
			
			timeStampArr[valIndex] = abs(lo);
			//timeStampArr[valIndex+1] = abs(lo+1);
			//timeStampArr[valIndex+2] = abs(lo+2);
			//timeStampArr[valIndex+3] = abs(lo+3);
			valIndex++;
			
			if(valIndex > 25){
				send_data_to_phone();
			}
			
		}
		
	}
	
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send!");
	sentFlag = 1;
}

static void out_succeded_handler(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sent! %i", heap_bytes_free());
	sentFlag = 1;
}

void handle_init(void) {
	// Create a window and text layer
	window = window_create();
	text_layer = text_layer_create(GRect(0, 0, 144, 154));
	
	// Set the text, font, and text alignment
	text_layer_set_text(text_layer, "Hi, I'm a Pebble!");
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	
	// Add the text layer to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));

	// Push the window
	window_stack_push(window, true);
	
	// Register message handlers
  app_message_register_outbox_failed(out_failed_handler);
	app_message_register_outbox_sent(out_succeded_handler);
  
  // Init buffers
  app_message_open(1024, 1024);
	
	valIndex = 0;
	for(int i=0;i<25;++i){
		valArr[i] = 0;
	}
	
	int num_samples = 25;
	accel_data_service_subscribe(num_samples, data_handler);
	accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
	
	// App Logging!
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}

void handle_deinit(void) {
	// Destroy the text layer
	text_layer_destroy(text_layer);
	accel_data_service_unsubscribe();
	// Destroy the window
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
