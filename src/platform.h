#if !defined(PLATFORM_H)
#define PLATFORM_H

// ui interface definition. Platforms perform the implementation
struct nk_context* platformCreateWindow(const char *title, int windowWidth, int windowHeight);
int platformRender(struct nk_colorf bg); 
void platformProcessInput(ApplicationState *ApplicationState);

// platform implementations
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "platform_windows.h"

#else

#endif


#endif // PLATFORM_H