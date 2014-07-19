// Author: Jim Kowalkowski
// Date: 2/96
// 
// $Id: gddNewDel.cc,v 1.1 1997-02-10 19:37:44 saunders Exp $
// 
// $Log: not supported by cvs2svn $
// Revision 1.1  1996/06/25 19:11:45  jbk
// new in EPICS base
//
//

// *Revision 1.2  1996/06/24 03:15:37  jbk
// *name changes and fixes for aitString and fixed string functions
// *Revision 1.1  1996/05/31 13:15:31  jbk
// *add new stuff

#include "gddNewDel.h"
#include <stdio.h>

gddCleanUp gddBufferCleanUp;

gddCleanUp::gddCleanUp(void) { }

gddCleanUp::~gddCleanUp(void) { gddCleanUp::CleanUp(); }

gddCleanUpNode* gddCleanUp::bufs = NULL;

void gddCleanUp::CleanUp(void)
{
	gddCleanUpNode *p1,*p2;

	for(p1=gddCleanUp::bufs;p1;)
	{
		p2=p1;
		p1=p1->next;
		free((char*)p2->buffer);
		delete p2;
	}
}

void gddCleanUp::Add(void* v)
{
	gddCleanUpNode* p = new gddCleanUpNode;
	p->buffer=v;
	p->next=gddCleanUp::bufs;
	gddCleanUp::bufs=p;
}

