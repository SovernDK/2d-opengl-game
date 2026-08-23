#pragma once
#include "scenes/graph_map_scene.h"

#include "SDL3/SDL.h"
#include "core/game.h"
#include "core/texts.h"
#include "core/resources.h"
#include "services/ui_service.h"

#include "rmui/ui_widget.h"
#include "gameplay/historia.h"

#include <imgui.h>
#include <globals/gdata.h>

using namespace rmui;
using namespace core;
using namespace ecs;
using namespace historia;

void GraphMapScene::start()
{
	IScene::start();

	ui = ServiceLocator::get<IUIService>();
	const auto& weakPtr = ui->widget("book_left_win");
	if (weakPtr.expired())
	{
		ErrorLog("GraphMapScene", "Failed to find right page window!");
		return;
	}

	leftPageWindow = weakPtr.lock();

	auto centerX = leftPageWindow->rect.pos.x + leftPageWindow->rect.size.x / 2;
	auto centerY = leftPageWindow->rect.pos.y + leftPageWindow->rect.size.y / 2;

	drawRoom(centerX, centerY, m_ctx->story()->currentRoom());
}

void GraphMapScene::update(float dt)
{
	IScene::update(dt);
}

void GraphMapScene::draw()
{
	IScene::draw();
}

void GraphMapScene::unload()
{
	IScene::unload();
	m_ctx->ecsWorld()->view<MapNode>([&](Entity& e, MapNode& m)
	{
		e.destroy();
	});
}

void GraphMapScene::quit()
{
	IScene::quit();
}

void GraphMapScene::drawRoom(float x, float y, const Room& room)
{
	std::set<RoomID> visited;
	std::stack<RoomID> queue;
	std::stack<glm::vec2> posQueue;

	queue.push(room.title);
	posQueue.push({ 0, 0 });

	auto pos = leftPageWindow->rect.pos;
	auto size = leftPageWindow->rect.size;

	glm::vec4 clip{ pos.x, pos.y, size.x, size.y };

	while (!queue.empty())
	{
		auto current = queue.top();
		auto pos = posQueue.top();
		queue.pop();
		posQueue.pop();

		visited.insert(current);

		float nextX = x + pos.x * 34;
		float nextY = y + pos.y * 34;

		auto& entityRoom = m_ctx->ecsWorld()->create()
			.add<Sprite>({ 
				.depth = 999,
				.isClipping = true,
				.clipping = clip
			})
			.add<Transform2D>({
				.position{ nextX, nextY }
				})
			.add<MapNode>({});

		for (auto& edge : m_ctx->story()->_graph[current])
		{
			if (!visited.contains(edge.toId))
			{
				queue.push(edge.toId);
				glm::vec2 newPos{ pos.x, pos.y };

				switch (edge.dir)
				{
				case EDirection::NORTH:
					newPos.y -= 1;
					break;
				case EDirection::EAST:
					newPos.x += 1;
					break;
				case EDirection::WEST:
					newPos.x -= 1;
					break;
				case EDirection::SOUTH:
					newPos.y += 1;
					break;
				}

				posQueue.push(newPos);
			}
		}
	}
}