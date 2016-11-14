#include <pebble.h>

#include "helper.h"

static Window *s_main_window;
static BitmapLayer *s_background_layer;
static Layer *s_analog_watch_layer;
static Layer *s_hands_layer;
static TextLayer *s_text_layer;

static GBitmap *s_background_bitmap;
static GFont s_text_font;

static GPath *s_tick_paths[NUM_CLOCK_TICKS];
static GPath *s_minute_arrow, *s_hour_arrow;

static void analogbg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, TICKS_COLOR);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    const int x_offset = PBL_IF_ROUND_ELSE(18, 0);
    const int y_offset = PBL_IF_ROUND_ELSE(6, 0);
    gpath_move_to(s_tick_paths[i], GPoint(x_offset, y_offset));
    gpath_draw_filled(ctx, s_tick_paths[i]);
  }
}

static void analoghands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  const int16_t second_hand_length = PBL_IF_ROUND_ELSE((bounds.size.w / 2) - 19, bounds.size.w / 2);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  GPoint second_hand = {
    .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
  };

  // second hand
  graphics_context_set_stroke_color(ctx, SECONDHAND_COLOR);
  graphics_draw_line(ctx, second_hand, center);

  // minute/hour hand
  graphics_context_set_fill_color(ctx, HANDS_COLOR);
  graphics_context_set_stroke_color(ctx, GColorBlack);

  gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
  gpath_draw_filled(ctx, s_minute_arrow);
  gpath_draw_outline(ctx, s_minute_arrow);

  gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
  gpath_draw_filled(ctx, s_hour_arrow);
  gpath_draw_outline(ctx, s_hour_arrow);

  // dot in the middle
  graphics_context_set_fill_color(ctx, DOT_COLOR);
  graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);
}

static void textlayer_update_proc() {
  // Get a tm structure
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

  // Copy date into buffer from tm structure
  static char date_buffer[8];
  strftime(date_buffer, sizeof(date_buffer), "%d %b", tick_time);
  
  // Show the date
  text_layer_set_text(s_text_layer, date_buffer);
  
  // Align the text layer vertically
  verticalAlignTextLayer(s_main_window, s_text_layer, AlignTextBottom);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(s_main_window));
  textlayer_update_proc();
}

void loadWatchLayers(Layer *window_layer) {
  GRect bounds = layer_get_bounds(window_layer);
  
  s_analog_watch_layer = layer_create(bounds);
  layer_set_update_proc(s_analog_watch_layer, analogbg_update_proc);
  layer_add_child(window_layer, s_analog_watch_layer);
  
  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, analoghands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATMAN_WHITE);

  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);
  
  // Create GFont
  s_text_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BATMAN_BLACK_16));

  // Create the TextLayer with specific bounds
  s_text_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

  // Improve the layout
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_font(s_text_layer, s_text_font);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);

  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  
  // Add text layer as a child layer to the Window's root layer (must be last to be on top)
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  
  // Add analog watch layer
  loadWatchLayers(window_layer);
}

void initWatchLayer(Layer *window_layer) {
  GRect bounds = layer_get_bounds(window_layer);
  
  // init hand paths
  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);
  
  GPoint center = grect_center_point(&bounds);
  gpath_move_to(s_minute_arrow, center);
  gpath_move_to(s_hour_arrow, center);

  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    s_tick_paths[i] = gpath_create(&ANALOG_BG_POINTS[i]);
  }
}

void unloadWatchLayer() {
  layer_destroy(s_analog_watch_layer);
  layer_destroy(s_hands_layer);
}

static void main_window_unload(Window *window) {
  // Destroy analog watch
  unloadWatchLayer();
  
  // Destroy TextLayer
  text_layer_destroy(s_text_layer);
  
  // Unload GFont
  fonts_unload_custom_font(s_text_font);
  
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Set the Window background color
  window_set_background_color(s_main_window, GColorWhite);
  
  Layer *window_layer = window_get_root_layer(s_main_window);
  //layer_set_update_proc(window_layer, layer_update_proc);
  
  initWatchLayer(window_layer);
  
  // Make sure everything is displayed from the start
  textlayer_update_proc();
  
  // Subscribe to time event
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

void deinitWatchLayer() {
  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow);

  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    gpath_destroy(s_tick_paths[i]);
  }
}

static void deinit() {
  // Destroy analog watch
  deinitWatchLayer();
  
  // Destroy Window
  window_destroy(s_main_window);
  
  // Unsubscribe time event
  tick_timer_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
