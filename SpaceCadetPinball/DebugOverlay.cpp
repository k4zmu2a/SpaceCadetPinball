#include "pch.h"
#include "DebugOverlay.h"

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


gdrv_bitmap8* DebugOverlay::dbScreen = nullptr;

int SDL_RenderDrawCircle(SDL_Renderer* renderer, int x, int y, int radius)
{
	int offsetx, offsety, d;
	int status;

	offsetx = 0;
	offsety = radius;
	d = radius - 1;
	status = 0;

	while (offsety >= offsetx) {
		status += SDL_RenderDrawPoint(renderer, x + offsetx, y + offsety);
		status += SDL_RenderDrawPoint(renderer, x + offsety, y + offsetx);
		status += SDL_RenderDrawPoint(renderer, x - offsetx, y + offsety);
		status += SDL_RenderDrawPoint(renderer, x - offsety, y + offsetx);
		status += SDL_RenderDrawPoint(renderer, x + offsetx, y - offsety);
		status += SDL_RenderDrawPoint(renderer, x + offsety, y - offsetx);
		status += SDL_RenderDrawPoint(renderer, x - offsetx, y - offsety);
		status += SDL_RenderDrawPoint(renderer, x - offsety, y - offsetx);

		if (status < 0) {
			status = -1;
			break;
		}

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

	// Draw all edges registered in TCollisionComponent.EdgeList + flippers
	if (options::Options.DebugOverlayAllEdges)
		DrawAllEdges();

	// Draw ball collision
	if (options::Options.DebugOverlayBallPosition || options::Options.DebugOverlayBallEdges)
		DrawBallInfo();

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
		vector2 boxPt{ x * edgeMan.AdvanceX + edgeMan.X , edgeMan.Y };
		auto pt1 = proj::xform_to_2d(boxPt);
		boxPt.Y = edgeMan.MaxBoxY * edgeMan.AdvanceY + edgeMan.Y;
		auto pt2 = proj::xform_to_2d(boxPt);

		SDL_RenderDrawLine(winmain::Renderer, pt1.X, pt1.Y, pt2.X, pt2.Y);
	}
	for (int y = 0; y <= edgeMan.MaxBoxY; y++)
	{
		vector2 boxPt{ edgeMan.X, y * edgeMan.AdvanceY + edgeMan.Y };
		auto pt1 = proj::xform_to_2d(boxPt);
		boxPt.X = edgeMan.MaxBoxX * edgeMan.AdvanceX + edgeMan.X;
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
				maths::vector_add(nextPos, maths::vector_mul(ball->Acceleration, ball->Speed / 10.0f));
				auto pt2 = proj::xform_to_2d(nextPos);
				SDL_RenderDrawLine(winmain::Renderer, pt1.X, pt1.Y, pt2.X, pt2.Y);
			}
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
		if (refBall != nullptr && (refBall->FieldFlag & edge->CollisionGroup) == 0)
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
		flip->set_control_points(pb::time_now);
		flip->build_edges_in_motion();

		DrawLineType(TFlipperEdge::lineA);
		DrawLineType(TFlipperEdge::lineB);
		DrawCicleType(TFlipperEdge::circlebase);
		DrawCicleType(TFlipperEdge::circleT1);
	}
}
