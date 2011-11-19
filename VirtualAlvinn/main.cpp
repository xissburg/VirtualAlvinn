//
//  main.m
//  VirtualAlvinn
//
//  Created by xiss burg on 11/18/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "Alvinn.h"

int main(int argc, char** argv)
{
	Alvinn alvinn;
    
	if(!alvinn.Initialize())
		return 1;
    
	alvinn.Run();
    
	return 0;
}
