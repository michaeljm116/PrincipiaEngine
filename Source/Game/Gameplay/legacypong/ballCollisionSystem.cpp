#include "ballCollisionSystem.h"
#include "../Audio/audioComponents.h"

BallCollisionSystem::BallCollisionSystem(){
		addComponentType<BallComponent>();
		addComponentType<CollisionComponent>();
}

void BallCollisionSystem::initialize() {
	ballMapper.init(*world);
	colMapper.init(*world);
}

void BallCollisionSystem::processEntity(artemis::Entity &e) {
	BallComponent* ball = ballMapper.get(e);
	CollisionComponent* colMap = colMapper.get(e);
	if (ball->timeSinceHit > 0.1f && colMap->collisions.size() > 0) {
		for (auto col : colMap->collisions) {
			if (col.second.state == CollisionState::Continue || col.second.state == CollisionState::Start) {
				col.second.state = CollisionState::Exit;
				artemis::Entity& colider = world->getEntity(col.first);
				NodeComponent* nc = (NodeComponent*)colider.getComponent<NodeComponent>();
				if (nc->tags & TAG_NONE)
					break;
				else if (nc->tags & TAG_PLAYER) {
					ball->hitType = HitType::Player;
					CharacterComponent* cc = (CharacterComponent*)colider.getComponent<CharacterComponent>();
					ball->speed = cc->hitPower;
					//ball->direction = glm::reflect(ball->direction, bul2glmVec3(col.second.norm));
					ball->direction = ballReflection(ball->direction, col.second.pos, (TransformComponent*)e.getComponent<TransformComponent>());
					ball->timeSinceHit = 0.f;
					//REFLECT
					AudioComponent* ac = (AudioComponent*)colider.getComponent<AudioComponent>();
					ac->play = true;
				}
				else if (nc->tags & TAG_WALL) {
					ball->hitType = HitType::Wall;
					ball->direction = glm::reflect(ball->direction, bul2glmVec3(col.second.norm));
					ball->timeSinceHit = 0.f;
					AudioComponent* ac = (AudioComponent*)colider.getComponent<AudioComponent>();
					ac->play = true;
					//REFLECT
				}
				else if (nc->tags & TAG_GOAL) {
					//ball->hitType = HitType::Goal;
					//ball->goalHit = (nc->name.at(nc->name.length() - 1) - '1');
					//SCORE
					BallScoreComponent* bsc = (BallScoreComponent*)nc->data->getComponent<BallScoreComponent>();
					bsc->gotHit = true;
					BallSpawnComponent* spawner = (BallSpawnComponent*)world->getSingleton()->getComponent<BallSpawnComponent>();
					spawner->state = BallSpawnState::Remove;
					spawner->removedBallID = e.getId();
					AudioComponent* ac = (AudioComponent*)colider.getComponent<AudioComponent>();
					ac->play = true;
				}
			}
		}
	}
	//you need an oncollisionexit thing
	ball->timeSinceHit += world->getDelta();
}

glm::vec3 BallCollisionSystem::bul2glmVec3(const btVector3& in) {
	return glm::vec3(in[0], in[1], in[2]);
}

//So the way ball colliisons work in pong is....
//a hit towards the center of the paddle is will do a perfect perpendicular reflection
//a hit towards the edges will angle it up and down more
//if 
glm::vec3 BallCollisionSystem::ballReflection(glm::vec3 balldir, btVector3 colpos, TransformComponent* tc)
{
	glm::vec3 newdir = balldir;	
	newdir.x = -newdir.x;
	float diff = tc->global.position.z - colpos.getZ();
	diff *= 2.f;
	diff = glm::clamp(diff, -.75f, .75f);
	newdir.z = diff;// / 0.5f;
	return newdir;
}
