#ifndef LOOKUP_HPP
#define LOOKUP_HPP

template<int N, typename T>
class Lookup{
private:
	T values[N];
public:
	template<typename F>
	constexpr Lookup(F function):
		values()
	{
		for(int i=0; i<N; ++i){
			values[i] = function(i);
		}
	}
	constexpr T get(int n) const{
		return values[n];
	}
};

#endif // LOOKUP_HPP