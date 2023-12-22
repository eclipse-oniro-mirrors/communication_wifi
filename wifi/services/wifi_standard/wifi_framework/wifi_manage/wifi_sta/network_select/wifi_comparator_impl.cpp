/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sstream>
#include "wifi_comparator_impl.h"
#include "network_selection_utils.h"
#include "wifi_logger.h"

namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_LABEL("WifiComparatorImpl")

WifiScorerComparator::WifiScorerComparator(const std::string &comparatorName)
    : comparatorName(comparatorName) {}

void WifiScorerComparator::AddScorer(const std::shared_ptr<IWifiScorer> &scorer)
{
    if (scorer) {
        scorers.emplace_back(scorer);
    }
}

void WifiScorerComparator::GetBestCandidates(const std::vector<NetworkCandidate *> &candidates,
                                             std::vector<NetworkCandidate *> &selectedCandidates)
{
    /* if candidates is empty, it is unnecessary to score the candidate */
    if (candidates.empty()) {
        return;
    }
    std::vector<NetworkCandidate *> bestNetworkCandidates; //bestNetworkCandidates have founded.
    std::vector<ScoreResult> bestNetworkScoreResults; //score records of bestNetworkCandidates.
    bestNetworkCandidates.emplace_back(candidates.at(0)); //at first,we assume the first one is the best.
    for (int i = 1; i < candidates.size(); ++i) {
        bool isWorseNetworkCandidate = false;
        auto networkCandidate = candidates.at(i);
        for (int j = 0; j < scorers.size(); j++) {
            auto scorer = scorers.at(j);
            ScoreResult scoreResult;
            if (bestNetworkScoreResults.size() <= j) {
                /*
                 * if current scorer does not score best networkCandidate, we should score the bestCandidate
                 * by current scorer at first.
                 */
                bestNetworkScoreResults.emplace_back(scoreResult);
                scorer->DoScore(*bestNetworkCandidates.front(), bestNetworkScoreResults.at(j));
            }
            scorer->DoScore(*networkCandidate, scoreResult); // score current networkCandidate.
            if (scoreResult.score > bestNetworkScoreResults.at(j).score) {
                /*
                 * if the score of current networkCandidate is better than the best networkCandidate, it means we found
                 * a better networkCandidate, then log the best networkCandidates which should be abandon. clear the
                 * vector for best networkCandidates and score records, then continue for the next network candidate .
                 */
                LogWorseSelectedCandidates(bestNetworkCandidates, *networkCandidate, bestNetworkScoreResults);
                bestNetworkScoreResults.erase(bestNetworkScoreResults.begin() + j, bestNetworkScoreResults.end());
                bestNetworkScoreResults.emplace_back(scoreResult);
                bestNetworkCandidates.clear();
                break;
            } else if (scoreResult.score < bestNetworkScoreResults.at(j).score) {
                /*
                 * if the score of current networkCandidate is worse than the best networkCandidate, log the msg of
                 * current network candidate, and continue the next networkCandidate.
                 */
                LogWorseCandidate(*networkCandidate, *bestNetworkCandidates.front(), scoreResult);
                isWorseNetworkCandidate = true;
                break;
            }
        }
        if (!isWorseNetworkCandidate) {
            /* if the current networkCandidate is not worse than the best, add it to bestNetworkCandidates. */
            bestNetworkCandidates.emplace_back(networkCandidate);
        }
    }
    LogSelectedCandidates(bestNetworkCandidates, bestNetworkScoreResults);
    selectedCandidates.insert(selectedCandidates.end(), bestNetworkCandidates.begin(), bestNetworkCandidates.end());
}

void WifiScorerComparator::LogSelectedCandidates(std::vector<NetworkCandidate *> &selectedCandidates,
                                                 std::vector<ScoreResult> &scoreResults)
{
    WIFI_LOGI("%{public}s get best candidates %{public}s which get scores %{public}s",
              comparatorName.c_str(),
              getAllNetworkCandidateMsg(selectedCandidates).c_str(),
              getAllScoreMsg(scoreResults).c_str());
}

void WifiScorerComparator::LogWorseSelectedCandidates(std::vector<NetworkCandidate *> &worseNetworkCandidates,
                                                      NetworkCandidate &betterNetworkCandidate,
                                                      std::vector<ScoreResult> &scoreResults)
{
    WIFI_LOGD("%{public}s find a better candidate %{public}s, "
              "abandon candidates %{public}s which get scores %{public}s",
              comparatorName.c_str(),
              NetworkSelectionUtils::GetNetworkCandidateInfo(betterNetworkCandidate).c_str(),
              getAllNetworkCandidateMsg(worseNetworkCandidates).c_str(),
              getAllScoreMsg(scoreResults).c_str());
}

void WifiScorerComparator::LogWorseCandidate(NetworkCandidate &worseNetworkCandidates,
                                             NetworkCandidate &selectedNetworkCandidate,
                                             ScoreResult &scoreResult)
{
    WIFI_LOGD("%{public}s find a worse candidate %{public}s which getScore %{public}s"
              ",and current best candidate is %{public}s",
              comparatorName.c_str(),
              NetworkSelectionUtils::GetNetworkCandidateInfo(worseNetworkCandidates).c_str(),
              NetworkSelectionUtils::GetScoreMsg(scoreResult).c_str(),
              NetworkSelectionUtils::GetNetworkCandidateInfo(selectedNetworkCandidate).c_str());
}

std::string WifiScorerComparator::getAllNetworkCandidateMsg(std::vector<NetworkCandidate *> &networkCandidates)
{
    std::stringstream scoreMsg;
    scoreMsg << "[";
    for (auto i = 0; i < networkCandidates.size(); i++) {
        scoreMsg << NetworkSelectionUtils::GetNetworkCandidateInfo(*networkCandidates.at(i));
        if (i < networkCandidates.size() - 1) {
            scoreMsg << ", ";
        }
    }
    scoreMsg << "]";
    return scoreMsg.str();
}

std::string WifiScorerComparator::getAllScoreMsg(std::vector<ScoreResult> &scoreResults)
{
    std::stringstream scoreMsg;
    scoreMsg << "[";
    for (auto i = 0; i < scoreResults.size(); i++) {
        scoreMsg << NetworkSelectionUtils::GetScoreMsg(scoreResults.at(i));
        if (i < scoreResults.size() - 1) {
            scoreMsg << ", ";
        }
    }
    scoreMsg << "]";
    return scoreMsg.str();
}
}
} // namespace OHOS::Wifi