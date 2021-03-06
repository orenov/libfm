// Copyright (C) 2010, 2011, 2012, 2013, 2014 Steffen Rendle
// Contact:   srendle@libfm.org, http://www.libfm.org/
//
// This file is part of libFM.
//
// libFM is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// libFM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libFM.  If not, see <http://www.gnu.org/licenses/>.
//
//
// fm_learn_sgd.h: Stochastic Gradient Descent based learning for
// classification and regression
//
// Based on the publication(s):
// - Steffen Rendle (2010): Factorization Machines, in Proceedings of the 10th
//   IEEE International Conference on Data Mining (ICDM 2010), Sydney,
//   Australia.

#ifndef FM_LEARN_SGD_ELEMENT_H_
#define FM_LEARN_SGD_ELEMENT_H_

#include "fm_learn_sgd.h"

class fm_learn_sgd_element: public fm_learn_sgd {
	public:
		virtual void init() {
			fm_learn_sgd::init();
		}

		virtual void learn(Data& train, Data& test, Data& validation) {
			fm_learn_sgd::learn(train, test, validation);
			int final_num_iter = 0;
			std::deque<double> scores;
			std::deque<fm_state*> states;
			std::cout << "SGD: DON'T FORGET TO SHUFFLE THE ROWS IN TRAINING DATA TO GET THE BEST RESULTS." << std::endl; 
			// SGD
			for (int i = 0; i < num_iter; i++) {
				double iteration_time = getusertime();
				for (train.data->begin(); !train.data->end(); train.data->next()) {
					double p = fm->predict(train.data->getRow(), sum, sum_sqr);
					double mult = 0;
					mult = -train.target(train.data->getRowIndex())*(1.0-1.0/(1.0+exp(-train.target(train.data->getRowIndex())*p)));				
					SGD(train.data->getRow(), mult, sum);
				}

				iteration_time = (getusertime() - iteration_time);
				double logloss_train = evaluate(train);
				double logloss_test = evaluate(test);
				double logloss_validation = evaluate(validation);
				final_num_iter++;

				bool isStop = true;
				if (early_stop) {
				  fm_state *current = new fm_state();
          current->w0 = this->fm->w0;
          current->w = this->fm->w;
          current->v = this->fm->v;
          current->num_factor = this->fm->num_factor;
          current->num_attribute = this->fm->num_attribute;
				  scores.push_back(logloss_validation);
				  states.push_back(current);
					if (scores.size() < (unsigned int) num_stop + 2) {
						isStop = false;
					} else {
						for (uint j = scores.size() - num_stop; j < scores.size(); j++) {
							if (scores.at(scores.size() - num_stop - 1) > scores.at(j)) {
								isStop = false;
							}
						}
						scores.pop_front();
					  states.pop_front();
					}
				}
				std::cout << "#Iter=" << std::setw(3) << i << "\tTrain=" << logloss_train << "\tTest=" << logloss_test << "\tValidation=" << logloss_validation << std::endl;
				if (early_stop && isStop) {
				  double logloss_final = scores.at(0);
				  this->fm->state = states.at(0);
				  std::cout << "Copying best state" << std::endl;
				  this->fm->apply_state();
					std::cout << "Early Stopping Activated on #iter" << (i - num_stop) << " Final quality: " << logloss_final << std::endl;
					break;
				}
			}
		}
};

#endif /*FM_LEARN_SGD_ELEMENT_H_*/
