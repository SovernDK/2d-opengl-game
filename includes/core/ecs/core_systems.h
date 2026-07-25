#pragma once
#include "ecs/ecs.h"
#include "ecs/base_components.h"
#include "core/profiler.h"
#include "graphics/rendering/canvas_2d.h"

#include "utility/utils.h"
#include "utility/feel.h"
#include "resources.h"

namespace ecs
{
	class CoreSystems
	{
	private:
		ECSWorld& m_world;
	public:
		CoreSystems(ECSWorld& world) : m_world(world)
		{

		}

		void init()
		{
			m_world.system<ParticleEmitter>([&](Entity& e, ParticleEmitter& emitter)
			{
				updateParticleEmitter(e, emitter);
			});

			m_world.system<Particle, Transform2D, Sprite>(
				[&](Entity& e, Particle& p, Transform2D& t, Sprite& s)
			{
				updateParticle(e, p, t, s);
			});
		}

		void updateParticleEmitter(Entity& entity, ParticleEmitter& emitter)
		{
			const float dt = core::Profiler::instance().getDeltaTime();

			emitter.emiting.step(dt);
			emitter.interval.step(dt);

			if (emitter.interval.isTimeout())
			{
				// Change randomization of position from random to Poisson Disc Sampling
				const float posX = random::Float(emitter.emitArea.x, emitter.emitArea.x + emitter.emitArea.z);
				const float posY = random::Float(emitter.emitArea.y, emitter.emitArea.y + emitter.emitArea.w);

				const float life = random::Float(emitter.lifeRange.x, emitter.lifeRange.y);
				const float scale = random::Float(emitter.startScale.x, emitter.startScale.y);

				const float vel = random::Float(emitter.startVelocity.x, emitter.startVelocity.y);
				const glm::vec2 velocity = emitter.direction * vel;

				m_world.create()
					.add<Sprite>({
						.texture = emitter.texture.id,
						.blend = BlendMode::Alpha,
						.size = emitter.size,
						.depth = SPRITE_Z
						})
					.add<Transform2D>({
						.position = { posX, posY },
						.scale = glm::vec2(scale)
						})
					.add<Particle>({
						.lifeTime{ life },
						.velocity = velocity,
						.alpha = emitter.startAlpha,
						.color = emitter.startColor,
						.velCurve = emitter.velCurve.get(),
						.scaleCurve = emitter.scaleCurve.get(),
						.alphaCurve = emitter.alphaCurve.get(),
						})
					.childOf(entity.id);

				emitter.interval.reset();
			}
		}
		void updateParticle(Entity& e, Particle& p, Transform2D& t, Sprite& s)
		{
			const float dt = core::Profiler::instance().getDeltaTime();
			p.lifeTime.step(dt);

			const float ntime = std::clamp(p.lifeTime.getTime() / p.lifeTime.getLength(), 0.0f, 1.0f);
			const float easing = p.velCurve->sample(ntime);
			t.position += (p.velocity * easing) * dt;

			const float scaleX = 0.5f + p.scale.x * p.scaleCurve->sample(ntime);
			const float scaleY = 0.5f + p.scale.y * p.scaleCurve->sample(ntime);
			t.scale = glm::vec2(scaleX, scaleY);

			s.color.r = p.color.r;
			s.color.g = p.color.g;
			s.color.b = p.color.b;

			s.color.a = p.alpha * p.alphaCurve->sample(ntime);

			if (p.lifeTime.isTimeout())
			{
				m_world.destroy(e);
			}
		}
	};
}