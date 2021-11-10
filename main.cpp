#include <ApplicationServices/ApplicationServices.h>

static bool get_best_display_mode(CGDirectDisplayID display, size_t width, size_t height, CGDisplayModeRef &mode);
static void print_display_modes(CGDirectDisplayID display);
static bool switch_display_mode(CGDirectDisplayID display, CGDisplayModeRef mode);

static void print_usage(const char *program_name);

int main(int argc, const char *argv[]) {
  if (argc == 1) {
    print_display_modes(kCGDirectMainDisplay);
    return 0;
  }

  if (argc < 3 || argc > 4) {
    print_usage(argv[0]);
    return 1;
  }

  bool has_error = false;
  int width_index = 1;
  int height_index = 2;
  if (argc == 4) {
    if (strcmp(argv[2], "x") != 0) {
      has_error = true;
    } else {
      height_index = 3;
    }
  }

  unsigned long width;
  unsigned long height;
  if (has_error ||
      !(width = strtoul(argv[width_index], nullptr, 10)) ||
      !(height = strtoul(argv[height_index], nullptr, 10))) {
    print_usage(argv[0]);
    return 1;
  }

  CGDisplayModeRef new_mode;
  if (!get_best_display_mode(kCGDirectMainDisplay, width, height, new_mode)) {
    fprintf(stderr, "Invalid resolution.\n");
    return 2;
  }

  if (!switch_display_mode(kCGDirectMainDisplay, new_mode)) {
    fprintf(stderr, "Failed to set resolution.\n");
    CFRelease(new_mode);
    return 1;
  }

  CFRelease(new_mode);

  return 0;
}

static CFArrayRef get_all_display_modes(CGDirectDisplayID display) {
  const void *keys[1] = {kCGDisplayShowDuplicateLowResolutionModes};
  const void *values[1] = {kCFBooleanTrue};
  CFDictionaryRef options = CFDictionaryCreate(nullptr, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFArrayRef modes = CGDisplayCopyAllDisplayModes(display, options);

  CFRelease(options);
  return modes;
}

static bool get_best_display_mode(CGDirectDisplayID display, size_t width, size_t height, CGDisplayModeRef &mode) {
  CFArrayRef modes = get_all_display_modes(display);

  CFIndex best_match = -1;
  for (auto i = 0; i < CFArrayGetCount(modes); ++i) {
    auto m = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
    if (!CGDisplayModeIsUsableForDesktopGUI(m)) {
      continue;
    }

    unsigned int mode_width = CGDisplayModeGetWidth(m);
    unsigned int mode_height = CGDisplayModeGetHeight(m);
    if (mode_width == width && mode_height == height) {
      best_match = i;
    } else if (best_match >= 0) {
      break;
    }
  }

  if (best_match < 0) {
    CFRelease(modes);
    return false;
  }

  mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, best_match);
  CFRetain(mode);
  CFRelease(modes);
  return true;
}

static void print_display_modes(CGDirectDisplayID display) {
  printf("Available resolutions:\n");
  CFArrayRef modes = get_all_display_modes(display);

  unsigned int last_width = 0;
  unsigned int last_height = 0;
  for (auto i = 0; i < CFArrayGetCount(modes); ++i) {
    auto m = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
    if (!CGDisplayModeIsUsableForDesktopGUI(m)) {
      continue;
    }

    unsigned int width = CGDisplayModeGetWidth(m);
    unsigned int height = CGDisplayModeGetHeight(m);
    if (width != last_width || height != last_height) {
      last_width = width;
      last_height = height;
      printf("%d x %d\n", width, height);
    }
  }
  CFRelease(modes);

  CGRect screenFrame = CGDisplayBounds(display);
  CGSize screenSize = screenFrame.size;
  printf("Current resolution: %d %d\n", static_cast<int>(screenSize.width), static_cast<int>(screenSize.height));
}

static bool switch_display_mode(CGDirectDisplayID display, CGDisplayModeRef mode) {
  CGDisplayConfigRef config;
  if (CGBeginDisplayConfiguration(&config) == kCGErrorSuccess) {
    CGConfigureDisplayWithDisplayMode(config, display, mode, nullptr);
    CGCompleteDisplayConfiguration(config, kCGConfigureForSession);
    return true;
  }
  return false;
}

static void print_usage(const char *program_name) {
  fprintf(stderr, "Usage %s <horizontal_resolution> [x] <vertical_resolution>\n", program_name);
  fprintf(stderr, "\n");
  print_display_modes(kCGDirectMainDisplay);
}
