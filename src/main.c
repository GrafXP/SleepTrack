#include <pebble.h>
#include <math.h>

//test

Window *window;
TextLayer *text_layer;
int sentFlag = 1;
int oldVal = 0;

uint32_t valArr[35];
uint32_t timeStampArr[35];
int valIndex = 0;
uint64_t firstTimestamp = 0;

static bool send_data_to_phone(){
	
	DictionaryIterator *iter;
	int i = 0;
	
	while(!sentFlag){
		
	}
	
	//testcomment
	
	
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
	for(i=0;i<25;++i){
		valArr[i] = 0;
		timeStampArr[i] = 0;
	}

  return true;
}

static void data_handler(AccelData *data, uint32_t num_samples){
	
	//int tmpValArr[25];
	unsigned i = 0;
	int valx = 0;
	int valy = 0;
	int valz = 0;
	uint32_t result = 0;
	uint64_t timestampDiff = 0;
	
	for(i = 1;i<num_samples;i++){
		
		if(firstTimestamp == 0){
			firstTimestamp = data[i].timestamp;
		}
		
		
		valx = abs(data[i].x - data[i-1].x);
		valy = abs(data[i].y - data[i-1].y);
		valz = abs(data[i].z - data[i-1].z);
		//valx = abs(data[i].x);
		//valy = abs(data[i].y);
		//valz = abs(data[i].z);
		
		if((valx+valy+valz) > 50){
			
			result = (uint32_t)(valx & 0b0000001111111111) << 20 | (uint32_t)(valy & 0b0000001111111111) << 10 | (uint32_t)(valz & 0b0000001111111111);
			
			valArr[valIndex] = result;

			timestampDiff =  data[i].timestamp - firstTimestamp;
			
			timeStampArr[valIndex] = (uint32_t)(timestampDiff & 0b0000000000000000000000000000000011111111111111111111111111111111);

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
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sent! %i", (int)app_message_outbox_size_maximum());
	sentFlag = 1;
}

void handle_init(void) {
	int i = 0;
	
	// Create a window and text layer
	window = window_create();
	text_layer = text_layer_create(GRect(0, 0, 144, 154));
	
	// Set the text, font, and text alignment
	text_layer_set_text(text_layer, "SleepTrack");
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
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	valIndex = 0;
	for(i=0;i<25;++i){
		valArr[i] = 0;
		timeStampArr[i] = 0;
	}
	
	int num_samples = 25;
	accel_data_service_subscribe(num_samples, data_handler);
	accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
	
	// App Logging!
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}

void handle_deinit(void) {
	while(!sentFlag){
		
	}
	app_message_deregister_callbacks();
	accel_data_service_unsubscribe();
	
	// Destroy the text layer
	text_layer_destroy(text_layer);
	
	// Destroy the window
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
