/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"

#include "clib/random.h"

#include "tree.h"

GTree::GTree()
{
    ++n_GTree;
}

GTree::~GTree()
{
    --n_GTree;
    while (!nodes.empty())
    {
        delete nodes.back();
        nodes.pop_back();
    }
}
void GTree::clone_nodes_onto( GTree* newtree ) const
{
    for( std::vector<GTree*>::const_iterator itr = nodes.begin();
         itr != nodes.end();
         ++itr )
    {
        if (*itr)
            newtree->nodes.push_back( (*itr)->clone() );
        else
            newtree->nodes.push_back( NULL );
    }
}


int GTree::num_children() const
{
    int count = 1;

    for( std::vector<GTree*>::const_iterator itr = nodes.begin();
         itr != nodes.end();
         ++itr )
    {
        count += (*itr)->num_children();
    }

    return count;
}


int GTree::max_depth() const
{
    int count = 1;
    int deepest = 0;
    for( std::vector<GTree*>::const_iterator itr = nodes.begin();
         itr != nodes.end();
         ++itr )
    {
        int depth = (*itr)->max_depth();
        if (depth > deepest)
            deepest = depth;
    }
    return deepest+count;
}

GTree** find_nth_child( GTree** ptree, int& n )
{
    if (n == 0)
        return ptree;

    for( std::vector<GTree*>::iterator itr = (*ptree)->nodes.begin();
         itr != (*ptree)->nodes.end();
         ++itr )
    {
        --n;
        if (n == 0)
            return &(*itr);
        if ( (*itr)->nodes.size() )
        {
            GTree** res = find_nth_child( &(*itr), n );
            if (res) return res;
        }
    }
    return NULL;
}

GTree** GTree::choose_random_tree(GTree** ptree)
{
    int count = (*ptree)->num_children();
    if (count < 10)
    {
        int nodenum = random_int( count );
        GTree** result = find_nth_child( ptree, nodenum );
        return result;
    }
    else if (random_float(1.0) < 0.10) // do a terminal crossover point
    {
        for (;;)
        {
            int nodenum = random_int( count );
            GTree** result = find_nth_child( ptree, nodenum );
            if ((*result)->nodes.size() == 0)
                return result;
        }
    }
    else                        // do an internal [function] crossover point
    {
        for (;;)
        {
            int nodenum = random_int( count );
            GTree** result = find_nth_child( ptree, nodenum );
            if ((*result)->nodes.size() != 0)
                return result;
        }
    }
}

void GTree::writeOn( std::ostream& os ) const
{
    for( unsigned i = 0; i < nodes.size(); i++ )
    {
        nodes[i]->writeOn( os );
    }
}

void GTree::printOn( std::ostream& os, int indentlevel ) const
{
    for( std::vector<GTree*>::const_iterator itr = nodes.begin();
         itr != nodes.end();
         ++itr )
    {
        (*itr)->printOn( os, indentlevel+2 );
    }
}

void GTree::printSourceOn( std::ostream& os, int indentlevel ) const
{
    for( std::vector<GTree*>::const_iterator itr = nodes.begin();
         itr != nodes.end();
         ++itr )
    {
        (*itr)->printSourceOn( os, indentlevel+2 );
    }
}

void GTree::printSourceOn( std::ostream& os ) const
{
    printSourceOn( os, 0 );
}

void GTree::printOn( std::ostream& os ) const
{
    printOn( os, 0 );
}


