#ifndef SIDEBAR_TYPES_H
#define SIDEBAR_TYPES_H

// Sidebar build categories - Pharaoh-style flyout panels grouping related
// build modes behind one main-sidebar icon instead of listing every mode
// as its own row (which stopped scaling once the mode list grew past ~12).
typedef enum {
  BUILD_CATEGORY_FARMING,
  BUILD_CATEGORY_MANSUS,
  BUILD_CATEGORY_DEV,
  BUILD_CATEGORY_COUNT
} BuildCategory;

// SidebarState
// ----------------------
typedef struct {
  int SIDEBAR_WIDTH;
  int SIDEBAR_HEIGHT;
  int SIDEBAR_X;
  int SIDEBAR_PADDING;
  int FONT_SIZE;
  float ICON_WIDTH;
  float ICON_HEIGHT;
  float si_h1;
  float si_h2;
  float si_road;
  float si_bridge;
  float si_farmer1;
  float si_farmer2;
  float si_ox;
  float si_oak;
  float si_wheat;
  float si_grass;
  float si_clear_forest;
  float si_mansus;
  float si_delete;
  float si_hive;
  float si_well;
  int open_category; // -1: no flyout open, else a BuildCategory index
  bool category_hovered[BUILD_CATEGORY_COUNT];
} SidebarState;


#endif
