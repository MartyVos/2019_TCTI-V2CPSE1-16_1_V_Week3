#include "hwlib.hpp"
#include "lookup.hpp"

constexpr double PI = 3.14159265359;

constexpr double pow(double g, double n){
	double result = 1;
	while(n > 0){
		result *= g;
		--n;
	}
	return result;
}

constexpr double fac(int n){
	double result = 1;
	while(n > 0){
		result *= n;
		--n;
	}
	return result;
}

constexpr double sin(double a){
	return
		-((a-PI)
		- pow(a-PI,3)/fac(3)
		+ pow(a-PI,5)/fac(5)
		- pow(a-PI,7)/fac(7)
		+ pow(a-PI,9)/fac(9)
		- pow(a-PI,11)/fac(11)
		+ pow(a-PI,13)/fac(13));
}

constexpr double rad_to_deg(int deg){
	return 2 * PI * (deg / 360.0);
}

void generate_positions(const Lookup<360, int> &table, hwlib::xy *positions, int size){
	int index = 0;
	int c = 360 / size;
	for(int i=0; i<size; i++){
		index = c * i;
		if(index+90 >= 360){
			positions[i] = hwlib::xy{table.get(index-270),table.get(index)};	
		}else{
			positions[i] = hwlib::xy{table.get(index+90),table.get(index)};	
		}
	}	
}

int main( void ){
	hwlib::wait_ms( 10 );
	
	constexpr auto table = Lookup<360, int>(
		[](int x)-> int {
			return 20 * (1.0 + sin(rad_to_deg(x)))+20;
		}
	);
	
	
	namespace target = hwlib::target;
	
	auto scl = hwlib::target::pin_oc{hwlib::target::pins::scl};
    auto sda = hwlib::target::pin_oc{hwlib::target::pins::sda};
	auto b_min = hwlib::target::pin_in{hwlib::target::pins::d24};
	auto b_hour = hwlib::target::pin_in{hwlib::target::pins::d25};
	auto i2c_bus  = hwlib::i2c_bus_bit_banged_scl_sda( scl, sda );
	auto oled_channel = i2c_bus.channel( 0x3C );
	auto oled = hwlib::glcd_oled( oled_channel );
	
	oled.clear();
	oled.flush();
	
	hwlib::xy min_pos[60];
	hwlib::xy hour_pos[12];
	
	hwlib::xy clock_pos[12] = {hwlib::xy{60,40}, hwlib::xy{57,50}, hwlib::xy{50,57},
							   hwlib::xy{40,60}, hwlib::xy{30,57}, hwlib::xy{22,50},
							   hwlib::xy{20,40}, hwlib::xy{22,30}, hwlib::xy{30,22},
							   hwlib::xy{40,20}, hwlib::xy{50,22}, hwlib::xy{57,30}
							   };
	oled.clear();
	generate_positions(table, min_pos, 60);
	generate_positions(table, hour_pos, 12);
	for(int i=0; i<12; i++){
			oled.write(clock_pos[i], hwlib::white);
	}
	
	while(true){
		int h = 0;
		int m = 0;
		uint_fast64_t T_NOW;
		
		while(h < 12){
			while(m < 60){
				if(b_min.read()){
					while(b_min.read()){}
					if(m == 59){
						m = 0;
					}else{
						m++;
					}
				}
				if(b_hour.read()){
					while(b_hour.read()){}
					if(h == 11){
						h = 0;
					}else{
						h++;
					}
				}
				
				T_NOW = hwlib::now_us();

				hwlib::line hour(hwlib::xy{40,40}, hour_pos[h], hwlib::white);
				hour.draw(oled);
				hwlib::line minute(hwlib::xy{40,40}, min_pos[m], hwlib::white);
				minute.draw(oled);
				oled.flush();
				
				while(!b_min.read() && !b_hour.read() && (T_NOW + 60'000'000) >= hwlib::now_us());
	
				hwlib::line minute2(hwlib::xy{40,40}, min_pos[m], hwlib::black);
				minute2.draw(oled);
				hwlib::line hour2(hwlib::xy{40,40}, hour_pos[h], hwlib::black);
				hour2.draw(oled);
				if(!b_min.read() && !b_hour.read()) m++;
			}
			m = 0;
			if(h < 12){
				h++;
			}else{
				h = 0;
			}
			hwlib::line hour2(hwlib::xy{40,40}, hour_pos[h], hwlib::black);
			hour2.draw(oled);
			oled.flush();
		}
	}	
}
