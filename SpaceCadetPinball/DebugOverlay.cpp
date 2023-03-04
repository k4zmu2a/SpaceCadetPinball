#include "pch.h"
#include "DebugOverlay.h"

#include "loader.h"
#include "maths.h"
#include "proj.h"
#include "winmain.h"
#include "TFlipperEdge.h"
#include "TFlipper.h"
#include "pb.h"
#include "TLine.h"
#include "TCircle.h"
#include "TPinballTable.h"
#include "TEdgeBox.h"
#include "TTableLayer.h"
#include "TBall.h"
#include "render.h"
#include "options.h"
#include "Sound.h"


gdrv_bitmap8* DebugOverlay::dbScreen = nullptr;

static int SDL_RenderDrawCircle(SDL_Renderer* renderer, int x, int y, int radius)
{
	SDL_Point points[256];
	int pointCount = 0;
	int offsetx, offsety, d;
	int status;

	offsetx = 0;
	offsety = radius;
	d = radius - 1;
	status = 0;

	while (offsety >= offsetx)
	{
		if (pointCount + 8 > 256)
		{
			status = SDL_RenderDrawPoints(renderer, points, pointCount);
			pointCount = 0;

			if (status < 0) {
				status = -1;
				break;
			}
		}

		points[pointCount++] = { x + offsetx, y + offsety };
		points[pointCount++] = { x + offsety, y + offsetx };
		points[pointCount++] = { x - offsetx, y + offsety };
		points[pointCount++] = { x - offsety, y + offsetx };
		points[pointCount++] = { x + offsetx, y - offsety };
		points[pointCount++] = { x + offsety, y - offsetx };
		points[pointCount++] = { x - offsetx, y - offsety };
		points[pointCount++] = { x - offsety, y - offsetx };

		if (d >= 2 * offsetx) {
			d -= 2 * offsetx + 1;
			offsetx += 1;
		}
		else if (d < 2 * (radius - offsety)) {
			d += 2 * offsety - 1;
			offsety -= 1;
		}
		else {
			d += 2 * (offsety - offsetx - 1);
			offsety -= 1;
			offsetx += 1;
		}
	}

	if (pointCount > 0)
		status = SDL_RenderDrawPoints(renderer, points, pointCount);

	return status;
}

void DebugOverlay::UnInit()
{
	delete dbScreen;
	dbScreen = nullptr;
}

void DebugOverlay::DrawOverlay()
{
	if (dbScreen == nullptr)
	{
		dbScreen = new gdrv_bitmap8(render::vscreen->Width, render::vscreen->Height, false, false);
		dbScreen->CreateTexture("nearest", SDL_TEXTUREACCESS_TARGET);
		SDL_SetTextureBlendMode(dbScreen->Texture, SDL_BLENDMODE_BLEND);
	}

	// Setup overlay rendering
	Uint8 initialR, initialG, initialB, initialA;
	auto initialRenderTarget = SDL_GetRenderTarget(winmain::Renderer);
	SDL_GetRenderDrawColor(winmain::Renderer, &initialR, &initialG, &initialB, &initialA);
	SDL_SetRenderTarget(winmain::Renderer, dbScreen->Texture);
	SDL_SetRenderDrawColor(winmain::Renderer, 0, 0, 0, 0);
	SDL_RenderClear(winmain::Renderer);

	// Draw EdgeManager box grid
	if (options::Options.DebugOverlayGrid)
		DrawBoxGrid();

	// Draw bounding boxes around sprites
	if (options::Options.DebugOverlaySprites)
		DrawAllSprites();

	// Draw all edges registered in TCollisionComponent.EdgeList + flippers
	if (options::Options.DebugOverlayAllEdges)
		DrawAllEdges();

	// Draw ball collision info
	if (options::Options.DebugOverlayBallPosition || options::Options.DebugOverlayBallEdges)
		DrawBallInfo();

	// Draw positions associated with currently playing sound channels
	if (options::Options.DebugOverlaySounds)
		DrawSoundPositions();

	// Draw ball depth cutoff steps that determine sprite size.
	if (options::Options.DebugOverlayBallDepthGrid)
		DrawBallDepthSteps();

	// Draw AABB of collision components
	if (options::Options.DebugOverlayAabb)
		DrawComponentAabb();

	// Restore render target
	SDL_SetRenderTarget(winmain::Renderer, initialRenderTarget);
	SDL_SetRenderDrawColor(winmain::Renderer,
		initialR, initialG, initialB, initialA);

	// Copy overlay with alpha blending
	SDL_BlendMode blendMode;
	SDL_GetRenderDrawBlendMode(winmain::Renderer, &blendMode);
	SDL_SetRenderDrawBlendMode(winmain::Renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderCopy(winmain::Renderer, dbScreen->Texture, nullptr, &render::DestinationRect);
	SDL_SetRenderDrawBlendMode(winmain::Renderer, blendMode);
}

void DebugOverlay::DrawBoxGrid()
{
	auto& edgeMan = *TTableLayer::edge_manager;

	SDL_SetRenderDrawColor(winmain::Renderer, 0, 255, 0, 255);
	for (int x = 0; x <= edgeMan.MaxBoxX; x++)
	{
		vector2 boxPt{ x * edgeMan.AdvanceX + edgeMan.MinX , edgeMan.MinY };
		auto pt1 = proj::xform_to_2d(boxPt);
		boxPt.Y = edgeMan.MaxBoxY * edgeMan.AdvanceY + edgeMan.MinY;
		auto pt2 = proj::xform_to_2d(boxPt);

		SDL_RenderDrawLine(winmain::Renderer, pt1.X, pt1.Y, pt2.X, pt2.Y);
	}
	for (int y = 0; y <= edgeMan.MaxBoxY; y++)
	{
		vector2 boxPt{ edgeMan.MinX, y * edgeMan.AdvanceY + edgeMan.MinY };
		auto pt1 = proj::xform_to_2d(boxPt);
		boxPt.X = edgeMan.MaxBoxX * edgeMan.AdvanceX + edgeMan.MinX;
		auto pt2 = proj::xform_to_2d(boxPt);

		SDL_RenderDrawLine(winmain::Renderer, pt1.X, pt1.Y, pt2.X, pt2.Y);
	}
}

void DebugOverlay::DrawAllEdges()
{
	SDL_SetRenderDrawColor(winmain::Renderer, 0, 200, 200, 255);
	for (auto cmp : pb::MainTable->ComponentList)
	{
		auto collCmp = dynamic_cast<TCollisionComponent*>(cmp);
		if (collCmp)
		{
			for (auto edge : collCmp->EdgeList)
			{
				DrawEdge(edge);
			}
		}
		auto flip = dynamic_cast<TFlipper*>(cmp);
		if (flip)
		{
			DrawEdge(flip->FlipperEdge);
		}
	}
}

void DebugOverlay::DrawBallInfo()
{
	auto& edgeMan = *TTableLayer::edge_manager;
	for (auto ball : pb::MainTable->BallList)
	{
		if (ball->ActiveFlag)
		{
			vector2 ballPosition = { ball->Position.X, ball->Position.Y };

			if (options::Options.DebugOverlayBallEdges)
			{
				SDL_SetRenderDrawColor(winmain::Renderer, 255, 0, 0, 255);
				auto x = edgeMan.box_x(ballPosition.X), y = edgeMan.box_y(ballPosition.Y);
				auto& box = edgeMan.BoxArray[x + y * edgeMan.MaxBoxX];
				for (auto edge : box.EdgeList)
				{
					DrawEdge(edge);
				}
			}

			if (options::Options.DebugOverlayBallPosition)
			{
				SDL_SetRenderDrawColor(winmain::Renderer, 0, 0, 255, 255);

				auto pt1 = proj::xform_to_2d(ballPosition);
				SDL_RenderDrawCircle(winmain::Renderer, pt1.X, pt1.Y, 10);

				auto nextPos = ballPosition;
				maths::vector_add(nextPos, maths::vector_mul(ball->Direction, ball->Speed / 10.0f));
				auto pt2 = proj::xform_to_2d(nextPos);
				SDL_RenderDrawLine(winmain::Renderer, pt1.X, pt1.Y, pt2.X, pt2.Y);
			}
		}
	}
}

void DebugOverlay::DrawAllSprites()
{
	SDL_SetRenderDrawColor(winmain::Renderer, 200, 200, 0, 255);
	for (auto cmp : pb::MainTable->ComponentList)
	{
		if (cmp->RenderSprite)
		{
			auto& bmpR = cmp->RenderSprite->BmpRect;
			if (bmpR.Width != 0 && bmpR.Height != 0)
			{
				SDL_Rect rect{ bmpR.XPosition, bmpR.YPosition, bmpR.Width, bmpR.Height };
				SDL_RenderDrawRect(winmain::Renderer, &rect);
			}
		}
	}
}

void DebugOverlay::DrawSoundPositions()
{
	auto& edgeMan = *TTableLayer::edge_manager;
	SDL_SetRenderDrawColor(winmain::Renderer, 200, 0, 200, 255);

	for (auto& posNorm : Sound::Channels)
	{
		auto pos3D = edgeMan.DeNormalizeBox(posNorm.Position);
		auto pos2D = proj::xform_to_2d(pos3D);
		SDL_RenderDrawCircle(winmain::Renderer, pos2D.X, pos2D.Y, 7);
	}
}

void DebugOverlay::DrawBallDepthSteps()
{
	auto& edgeMan = *TTableLayer::edge_manager;
	SDL_SetRenderDrawColor(winmain::Renderer, 200, 100, 0, 255);

	for (auto ball : pb::MainTable->BallList)
	{
		auto visualCount = loader::query_visual_states(ball->GroupIndex);
		for (auto index = 0; index < visualCount; ++index)
		{
			auto depthPt = reinterpret_cast<vector3*>(loader::query_float_attribute(ball->GroupIndex, index, 501));
			auto pt = proj::xform_to_2d(*depthPt);

			// Snap X coordinate to edge box sides
			auto x1 = proj::xform_to_2d(vector2{edgeMan.MinX, depthPt->Y}).X;
			auto x2 = proj::xform_to_2d(vector2{edgeMan.MaxBoxX * edgeMan.AdvanceX + edgeMan.MinX, depthPt->Y}).X;
			auto ff =  proj::xform_to_2d(vector2{ edgeMan.MaxBoxX * edgeMan.AdvanceX + edgeMan.MinX, depthPt->Y });
			SDL_RenderDrawLine(winmain::Renderer, x1, pt.Y, x2, pt.Y);
		}
		break;
	}
}

void DebugOverlay::DrawComponentAabb()
{
	SDL_SetRenderDrawColor(winmain::Renderer, 0, 50, 200, 255);
	for (auto cmp : pb::MainTable->ComponentList)
	{
		auto collCmp = dynamic_cast<TCollisionComponent*>(cmp);
		if (collCmp)
		{
			const auto& aabb = collCmp->AABB;
			auto pt1 = proj::xform_to_2d(vector2{ aabb.XMax, aabb.YMax });
			auto pt2 = proj::xform_to_2d(vector2{ aabb.XMin, aabb.YMin });
			SDL_Rect rect{ pt2.X,pt2.Y, pt1.X - pt2.X , pt1.Y - pt2.Y };
			SDL_RenderDrawRect(winmain::Renderer, &rect);
		}
	}
}

void DebugOverlay::DrawCicleType(circle_type& circle)
{
	vector2 linePt{ circle.Center.X + sqrt(circle.RadiusSq), circle.Center.Y };
	auto pt1 = proj::xform_to_2d(circle.Center);
	auto pt2 = proj::xform_to_2d(linePt);
	auto radius = abs(pt2.X - pt1.X);

	SDL_RenderDrawCircle(winmain::Renderer, pt1.X, pt1.Y, radius);
}

void DebugOverlay::DrawLineType(line_type& line)
{
	auto pt1 = proj::xform_to_2d(line.Origin);
	auto pt2 = proj::xform_to_2d(line.End);

	SDL_RenderDrawLine(winmain::Renderer, pt1.X, pt1.Y, pt2.X, pt2.Y);
}

void DebugOverlay::DrawEdge(TEdgeSegment* edge)
{
	if (options::Options.DebugOverlayCollisionMask)
	{
		TBall* refBall = nullptr;
		for (auto ball : pb::MainTable->BallList)
		{
			if (ball->ActiveFlag)
			{
				refBall = ball;
				break;
			}
		}
		if (refBall != nullptr && (refBall->CollisionMask & edge->CollisionGroup) == 0)
			return;
	}

	auto line = dynamic_cast<TLine*>(edge);
	if (line)
	{
		DrawLineType(line->Line);
		return;
	}

	auto circle = dynamic_cast<TCircle*>(edge);
	if (circle)
	{
		DrawCicleType(circle->Circle);
		return;
	}

	auto flip = dynamic_cast<TFlipperEdge*>(edge);
	if (flip)
	{
		if (flip->ControlPointDirtyFlag)
			flip->set_control_points(flip->CurrentAngle);

		DrawLineType(flip->LineA);
		DrawLineType(flip->LineB);
		DrawCicleType(flip->circlebase);
		DrawCicleType(flip->circleT1);
	}
}
