#define GUI_WIDTH 30
#define GUI_HEIGHT 30

#define GUI_BLACK 0x000000FF
#define GUI_WHITE 0xFFFFFFFF
#define GUI_RED 0xFF0000FF
#define GUI_GREEN 0x00FF00FF
#define GUI_BLUE 0x0000FFFF
#define GUI_AQUA 0x00FFFFFF
#define GUI_YELLOW 0xFFFF00FF
#define GUI_PURPLE 0xFF00FFFF

#define gui_rand() __builtin_riscs_rand()
#define gui_set_pixel(X, Y, Z) __builtin_riscs_set_pixel(X, Y, Z)
#define gui_flush() __builtin_riscs_flush()
#define gui_quit_event() __builtin_riscs_quit_event()

// extern int gui_rand(void);
// extern void gui_set_pixel(int, int, int);
// extern void gui_flush(void);
// extern int gui_quit_event(void);

void set_bound_cond(char layer[GUI_WIDTH]) {
  for (int k = 0; k != GUI_WIDTH; ++k) {
    int need_set = gui_rand() % 2;
    if (need_set)
      layer[k] = 1;
    else
      layer[k] = 0;
  }
}

void apply_rule(char prev[GUI_WIDTH], char next[GUI_WIDTH]) {
  for (int k = 0; k != GUI_WIDTH; ++k) {
    int neighbors = 0;
    if (prev[(k - 1 + GUI_WIDTH) % GUI_WIDTH])
      neighbors += 4;
    if (prev[k])
      neighbors += 2;
    if (prev[(k + 1) % GUI_WIDTH])
      neighbors += 1;
    if (110 & (1 << neighbors))
      next[k] = 1;
    else
      next[k] = 0;
  }
}

void app() {
  char data[GUI_HEIGHT][GUI_WIDTH];
  for (int i = 0; i != GUI_HEIGHT; ++i)
    for (int j = 0; j != GUI_WIDTH; ++j)
      data[i][j] = 0;
  set_bound_cond(data[0]);
  int cur = 0;
  for (;;) {
    if (gui_quit_event())
      break;
    apply_rule(data[cur % GUI_HEIGHT], data[(cur + 1) % GUI_HEIGHT]);
    for (int i = 0; i != GUI_HEIGHT; ++i) {
      for (int j = 0; j != GUI_WIDTH; ++j) {
        if (data[i][j])
          gui_set_pixel(j, i, GUI_BLACK);
        else
          gui_set_pixel(j, i, GUI_WHITE);
      }
    }
    gui_flush();
    cur++;
  }
}
