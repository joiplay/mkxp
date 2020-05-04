#include "emscripten.hpp"
#include "filesystem.h"
#include "sharedstate.h"

EM_JS(void, load_file_async_js, (const char* fullPathC), {
	Asyncify.handleSleep(function(wakeUp) {
		const fullPath = UTF8ToString(fullPathC);

		// Make cache object
		if (!window.fileAsyncCache) window.fileAsyncCache = {};

		// Check if already loaded
		if (window.fileAsyncCache.hasOwnProperty(fullPath)) return wakeUp();

		// Get full destination
		const file = "game/" + fullPath;

		// Get path and filename
		const path = "/" + file.substring(0, file.lastIndexOf("/"));
		const filename = file.substring(file.lastIndexOf("/") + 1);

		// Get target URL
		const iurl = "gameasync/" + fullPath;

		// Delete original file
		FS.unlink(path + "/" + filename);

		// Get the new file
		FS.createPreloadedFile(path, filename, iurl, true, true, function() {
			window.fileAsyncCache[fullPath] = 1;
			wakeUp();
		}, console.error);
	});
});

struct LoadOpenHandler : FileSystem::OpenHandler
{
	LoadOpenHandler()
	{}

	bool tryRead(SDL_RWops &ops, const char *ext, const char *fullPath)
	{
		load_file_async_js(fullPath);
		return true;
	}
};

void load_file_async(const char * filename) {
	LoadOpenHandler handler;
	shState->fileSystem().openRead(handler, filename);
}

