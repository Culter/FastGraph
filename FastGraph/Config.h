//
//  Config.h
//  FastGraph
//
//  Created by culter on 2/3/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef Config_h
#define Config_h

// Basic strategy choices
constexpr bool k_close_all_genes = false;

// Knobs to tune
constexpr int k_random_seed = 123456;
constexpr int k_refresh_edge_count = 0;
constexpr int k_population_multiplier = 1;

// Cosmetic changes
constexpr bool k_print_records = true;
constexpr int k_report_record_period = 100;
constexpr uint32_t k_max_generations = 5000; //UINT32_MAX;

#endif /* Config_h */
