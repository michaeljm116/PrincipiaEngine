#include "ballScoreSystem.h"

BallScoreSystem::BallScoreSystem() {
	addComponentType<BallScoreComponent>();
	addComponentType<GUINumberComponent>();
}

void BallScoreSystem::initialize() {
	scorer.init(*world);
	guiMapper.init(*world);
}

void BallScoreSystem::processEntity(artemis::Entity &e) {
	BallScoreComponent* score = scorer.get(e);
	if (score->gotHit) {
		score->gotHit = false;
		score->score++;

		GUINumberComponent* num = guiMapper.get(e);
		num->number = score->score;

		RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();
		rs->updateGuiNumber(num);


		//if (ball->hitType == HitType::Goal) {
		//	scoreComp->scores[ball->goalHit]++;
		//	spawner->state = BallSpawnState::Remove;
		//	spawner->removedBallID = e.getId();
		//}
	}
}