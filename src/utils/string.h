#ifndef _ES_RGMF_UTILS_STRING_H
#define _ES_RGMF_UTILS_STRING_H 1

#include <string>

namespace fitgalgo
{

/**
 * https://en.wikipedia.org/wiki/UTF-8
 * https://stackoverflow.com/questions/33903342/how-to-ignore-accents-in-a-string-so-it-does-not-alter-its-length
 */
inline size_t mb_strlen(const std::string& s)
{
    auto itr = s.begin();
    size_t n = 0;

    while (itr != s.end())
    {
	unsigned char c = *itr;

	if ((c & 0xc0) != 0x80)
	    n++;

	itr++;
    }

    return n;
}

} // namespace fitgalgo

#endif // _ES_RGMF_UTILS_STRING_H
