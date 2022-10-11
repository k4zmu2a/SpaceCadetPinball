#pragma once

struct gdrv_bitmap8;
struct circle_type;
struct line_type;
class TEdgeSegment;

class DebugOverlay
{
public:
	static void UnInit();
	static void DrawOverlay();
private:
	static gdrv_bitmap8* dbScreen;

	static void DrawCicleType(circle_type& circle);
	static void DrawLineType(line_type& line);
	static void DrawEdge(TEdgeSegment* edge);
	static void DrawBoxGrid();
	static void DrawAllEdges();
	static void DrawBallInfo();
	static void DrawAllSprites();
	static void DrawSoundPositions();
	static void DrawBallDepthSteps();
};