#ifndef TRIPLE_H
#define TRIPLE_H
#include "Element.h"

class Triple {
private:
	Element root;
	
	
public:
	Triple(const Element&&);
	Triple(const Triple&) = delete;


};

#endif //TRIPLE_H