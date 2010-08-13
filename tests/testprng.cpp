/*  PRNG testing
 *
 *  Copyright (C) 2010  Lee Begg and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <iostream>
#include <map>
#include "tpserver/prng.h"

int main(int argc, char** argv){
    Random random;
    std::cout << "Random tests" << std::endl << std::endl;
    
    random.seed(12345);
    
    for(int i = 0; i < 100; i++){
        std::cout << random.getInt32() << " ";
        if((i+1) % 8 == 0){
            std::cout << std::endl;
        }
    }
    
    std::cout << std::endl << std::endl;
    
    for(int i = 0; i < 100; i++){
        std::cout << random.getReal1() << " ";
        if((i+1) % 8 == 0){
            std::cout << std::endl;
        }
    }
    
    std::cout << std::endl << std::endl;
    
    for(int i = 0; i < 100; i++){
        std::cout << random.getReal2() << " ";
        if((i+1) % 8 == 0){
            std::cout << std::endl;
        }
    }
    
    std::cout << std::endl << std::endl;
    
    for(int i = 0; i < 100; i++){
        std::cout << random.getInRange(0,5) << " ";
        if((i+1) % 8 == 0){
            std::cout << std::endl;
        }
    }
    
    std::cout << std::endl << std::endl;
    
    std::map<int, int> values;
    for(int i = 0; i < 6; i++){
        values[i] = 0;
    }
    
    for(int i = 0; i < 1000000; i++){
        values[random.getInRange(0,5)] += 1;
    }
    
     for(int i = 0; i < 6; i++){
        std::cout << i << " " << values[i] << std::endl;
    }
    
    std::cout << std::endl << std::endl;
    
    return 0;
}
