#include "draw.h"

void GeneralDrawer::clear() {
	triangles_mm.clear();
	lines_mm.clear();
}
void GeneralDrawer::updateGPU() {
	triangles.uploadCPUtoGPU();
	lines.uploadCPUtoGPU();
}