#include "Display.h"
#include "Log.h"
Display display;

static uint32_t getRandomColor(uint32_t range)
{
    return tal_system_get_random(range);
}

static void printSystemInfo(void)
{
    PR_NOTICE("========================================");
    PR_NOTICE("Display Example");
    PR_NOTICE("Project name:        %s", PROJECT_NAME);
    PR_NOTICE("App version:         %s", PROJECT_VERSION);
    PR_NOTICE("Compile time:        %s %s", __DATE__, __TIME__);
    PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
    PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("========================================");
}

void setup() {
    OPERATE_RET rt = OPRT_OK;
    
    // Initialize logging
    Serial.begin(115200);
    Log.begin();
    
    printSystemInfo();

    // Initialize display
    rt = display.begin();
    if (rt != OPRT_OK) {
        PR_ERR("Failed to initialize display: %d", rt);
        return;
    }

    PR_NOTICE("Display initialized successfully");
    PR_NOTICE("Display resolution: %dx%d", display.getWidth(), display.getHeight());
    PR_NOTICE("Pixel format: %d", display.getPixelFormat());

    // Clear screen with black
    display.clear(0x000000);
    display.flush();
}

void loop() {
    OPERATE_RET rt = OPRT_OK;
    
    // Fill screen with random color
    uint32_t color = getRandomColor(0xFFFFFFFF);
    rt = display.fillScreen(color);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to fill screen: %d", rt);
    }

    // Flush to display
    rt = display.flush();
    if (rt != OPRT_OK) {
        PR_ERR("Failed to flush display: %d", rt);
    }
    
    delay(2000);
}