		case 20:{ //接口20，蜂鸣发声器
			if (eax == 0){
				data = io_in8(0x61);
				io_out8(0x61, data & 0x0d);
			}
			else{
				data = 1193180000 / eax;
				io_out8(0x43, 0xb6);
				io_out8(0x42, data & 0xff);
				io_out8(0x42, data >> 8);
				data = io_in8(0x61);
				io_out8(0x61, (data | 0x03) & 0x0f);
			}
			break;
		}