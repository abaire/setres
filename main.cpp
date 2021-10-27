#include <ApplicationServices/ApplicationServices.h>

static bool get_best_display_mode(CGDirectDisplayID display, size_t width, size_t height, CGDisplayModeRef &mode);
static void print_display_modes(CGDirectDisplayID display);
static bool switch_display_mode(CGDirectDisplayID display, CGDisplayModeRef mode);

int main(int argc, const char *argv[]) {
  if (argc == 1) {
    print_display_modes(kCGDirectMainDisplay);
    return 0;
  }

  unsigned long width;
  unsigned long height;
  if (argc != 3 ||
      !(width = strtoul(argv[1], nullptr, 10)) ||
      !(height = strtoul(argv[2], nullptr, 10))) {

    fprintf(stderr, "Usage %s <horizontal_resolution> <vertical_resolution>\n", argv[0]);
    fprintf(stderr, "\n");
    print_display_modes(kCGDirectMainDisplay);
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

static bool get_best_display_mode(CGDirectDisplayID display, size_t width, size_t height, CGDisplayModeRef &mode) {
  CFArrayRef modes = CGDisplayCopyAllDisplayModes(display, nullptr);

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
  CFArrayRef modes = CGDisplayCopyAllDisplayModes(display, nullptr);

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
