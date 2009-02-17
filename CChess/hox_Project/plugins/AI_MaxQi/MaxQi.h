#ifndef __INCLUDED_MAX_QI_H__
#define __INCLUDED_MAX_QI_H__

#include <string>

namespace MaxQi
{
	/* PUBLIC API */

	void initialize(unsigned char pcsSavedPos[][9]=NULL);
	std::string generate_move();
    void        on_human_move( const std::string& sMove );

} // namespace MaxQi

#endif /* __INCLUDED_MAX_QI_H__ */
