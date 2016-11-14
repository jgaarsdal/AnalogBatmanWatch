#include <pebble.h>

#include "helper.h"

void verticalAlignTextLayer(Window *window, TextLayer *layer, VerticalTextAlignment alignment) {
  GSize content = text_layer_get_content_size(layer);
  GRect bounds = layer_get_frame(window_get_root_layer(window));
  
  switch(alignment) {
    case AlignTextTop:
      layer_set_frame(text_layer_get_layer(layer),
             GRect(bounds.origin.x + (bounds.size.w - content.w) / 2, bounds.origin.y + 5, 
             content.w, content.h));
      break;
    case AlignTextBottom:
      layer_set_frame(text_layer_get_layer(layer),
             GRect(bounds.origin.x + (bounds.size.w - content.w) / 2, bounds.origin.y + (bounds.size.h - content.h - 34), 
             content.w, content.h));
      break;
    default: // Align center
      layer_set_frame(text_layer_get_layer(layer),
             GRect(bounds.origin.x + (bounds.size.w - content.w) / 2, bounds.origin.y + (bounds.size.h - content.h - 5) / 2, 
             content.w, content.h));
      break;
  }
}