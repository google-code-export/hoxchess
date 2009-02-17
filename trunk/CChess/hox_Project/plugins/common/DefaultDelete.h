/**
 * DLL Interface classes for C++ Binary Compatibility article
 * article at http://aegisknight.org/cppinterface.html
 *
 * article author:  Chad Austin (aegis@aegisknight.org)
 */

#ifndef __INCLUDED_DEFAULT_DELETE_H__
#define __INCLUDED_DEFAULT_DELETE_H__

template<typename T>
class DefaultDelete : public T
{
public:
    void operator delete(void* p)
    {
        ::operator delete(p);
    }
};

#endif /* __INCLUDED_DEFAULT_DELETE_H__ */
