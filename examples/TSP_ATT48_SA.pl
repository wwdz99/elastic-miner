/******************************************************************************
 *
 * Travelling Salesman Problem - Demo 2
 *
 * Name: TSP_ATT48_BF
 * Desc: 48 Captitals Of The US
 *       Using Pseudo-Euclidean distance
 *       Simulated Annealing Solution
 *
 * Memory Map:
 *   Random Inputs:       m[0] - m[11]
 *   Initialization Vars: m[20] - m[29]
 *   Initialization Data: m[60000] - m[60100]
 *   Cost Matrix:         m[10000] - m[59000]
 *   Path Vars:           m[1000] - m[9999]
 *
 *****************************************************************************/

// Initialize TSP Point Data
m[60000] = 6734;
m[60001] = 1453;
m[60002] = 2233;
m[60003] = 10;
m[60004] = 5530;
m[60005] = 1424;
m[60006] = 401;
m[60007] = 841;
m[60008] = 3082;
m[60009] = 1644;
m[60010] = 7608;
m[60011] = 4458;
m[60012] = 7573;
m[60013] = 3716;
m[60014] = 7265;
m[60015] = 1268;
m[60016] = 6898;
m[60017] = 1885;
m[60018] = 1112;
m[60019] = 2049;
m[60020] = 5468;
m[60021] = 2606;
m[60022] = 5989;
m[60023] = 2873;
m[60024] = 4706;
m[60025] = 2674;
m[60026] = 4612;
m[60027] = 2035;
m[60028] = 6347;
m[60029] = 2683;
m[60030] = 6107;
m[60031] = 669;
m[60032] = 7611;
m[60033] = 5184;
m[60034] = 7462;
m[60035] = 3590;
m[60036] = 7732;
m[60037] = 4723;
m[60038] = 5900;
m[60039] = 3561;
m[60040] = 4483;
m[60041] = 3369;
m[60042] = 6101;
m[60043] = 1110;
m[60044] = 5199;
m[60045] = 2182;
m[60046] = 1633;
m[60047] = 2809;
m[60048] = 4307;
m[60049] = 2322;
m[60050] = 675;
m[60051] = 1006;
m[60052] = 7555;
m[60053] = 4819;
m[60054] = 7541;
m[60055] = 3981;
m[60056] = 3177;
m[60057] = 756;
m[60058] = 7352;
m[60059] = 4506;
m[60060] = 7545;
m[60061] = 2801;
m[60062] = 3245;
m[60063] = 3305;
m[60064] = 6426;
m[60065] = 3173;
m[60066] = 4608;
m[60067] = 1198;
m[60068] = 23;
m[60069] = 2216;
m[60070] = 7248;
m[60071] = 3779;
m[60072] = 7762;
m[60073] = 4595;
m[60074] = 7392;
m[60075] = 2244;
m[60076] = 3484;
m[60077] = 2829;
m[60078] = 6271;
m[60079] = 2135;
m[60080] = 4985;
m[60081] = 140;
m[60082] = 1916;
m[60083] = 1569;
m[60084] = 7280;
m[60085] = 4899;
m[60086] = 7509;
m[60087] = 3239;
m[60088] = 10;
m[60089] = 2676;
m[60090] = 6807;
m[60091] = 2993;
m[60092] = 5185;
m[60093] = 3258;
m[60094] = 3023;
m[60095] = 1942;

m[20] = 48;	// Number of TSP Points
m[21] = 60000; 	// Starting index of Point Data
m[22] = 10000;	// Starting index of Cost Matrix
m[23] = m[21];	// Index of x[i] - Point A
m[24] = m[21];  // Index of y[i] - Point B
m[25] = m[22];	// Index of dij - Cost Matrix Element

// Create TSP Cost Matrix
repeat(m[20]) {

	repeat(m[20]) {

		if(m[24] == m[23]) {
			m[24] += 2;   // Move To Next Point B
			m[m[25]] = 0; // Distance To Self Is Zero
			m[25]++;      // Move To Next Matrix Element
			continue;
		}
			
		m[26] = m[m[24]] - m[m[23]];				// xd
		m[27] = m[m[24] + 1] - m[m[23] + 1];			// yd
		f[0] = sqrt(((m[26]*m[26]) + (m[27]*m[27])) / 10.0);	// rij
		m[28] = (f[0] + 0.5);					// tij - equivalent of nint function
		if (m[28] < f[0])
			m[m[25]] = m[28] + 1;				// dij
		else
			m[m[25]] = m[28];				// dij
		
		m[24] += 2; // Move To Next Point B
		m[25]++;    // Move To Next Matrix Element
	}
	
	m[23] += 2;	// Increment Point A
	m[24] = m[21];	// Reset Point B
}

// Initialize The Path
m[30] = 50; // Index of Path
m[31] = 0;  // Counter
repeat(m[20]) {
	m[m[30] + m[31]] = m[31]++;
}

// Randomize The Path
m[m[30]] = 0;      // First Value Always Zero
m[m[30] + m[20]] = 0;  // Last Value Always Zero
m[31] = m[20] - 1; // Start With Final Point In Path
repeat(m[20] - 1) {
	m[32] = (abs(m[0]) % m[31]) + 1; // Use m[0] for random input
	m[33] = m[m[30] + m[32]];
	m[m[30] + m[32]] = m[m[30] + m[31]];
	m[m[30] + m[31]] = m[33];
	m[31]--;
}

// Simulated Annealing Variables
m[40] = 10000; // Max Num Iterations
m[41] = 0;     // Random City 1
m[42] = 0;     // Random City 2
m[43] = 0;     // Temp Variable To Swap City
m[44] = 0;     // Current Distance
m[45] = 0;     // New Distance
m[46] = abs(m[1]); // Random #1
m[47] = abs(m[2]); // Random #2
f[1] = 10000.0;// Starting Temp
f[2] = 0.003;  // Cooling Rate
f[3] = 0.1;    // Min Temp
f[4] = 0.0;    // Acceptance Probability
f[5] = 0.0;    // Random Number

// Simulated Annealing Logic
repeat(m[40]) {
	
	// Update Temp
	f[1] *= 1-f[2];
	if (f[1] <= f[3])
		break;

	// Sum Current Distance
	m[31] = 0; // Counter
	m[44] = 0; // Total Distance
	repeat(m[20]) {
		m[34] = m[m[30] + m[31]];     // Matrix Row
		m[35] = m[m[30] + m[31] + 1]; // Matrix Column
		m[44] += m[(m[22] + (m[34] * m[20]) + m[35])];
		m[31]++;
	}
	
	// Randomize The Numbers
	m[46] = abs((m[46] <<< 1) ^ m[2]);
	m[47] = abs((m[47] <<< 1) ^ m[2]);
	
	// Pick Random Cities & Swap
	m[41] = m[46] % m[20];               // Random City 1
	m[42] = m[47] % m[20];               // Random City 2
	m[43] = m[m[30] + m[41]];            // City 1 Number
	m[m[30] + m[41]] = m[m[30] + m[42]]; // Swap City 1
	m[m[30] + m[42]] = m[43];            // Swap City 2

	// Sum New Distance
	m[31] = 0; // Counter
	m[45] = 0; // Total Distance
	repeat(m[20]) {
		m[34] = m[m[30] + m[31]];     // Matrix Row
		m[35] = m[m[30] + m[31] + 1]; // Matrix Column
		m[45] += m[(m[22] + (m[34] * m[20]) + m[35])];
		m[31]++;
	}
	
	// If New Route Is Shorter, Keep It
	if (m[45] < m[44])
		continue;
	
	// Calculate Acceptance Probability
	f[4] = exp((m[44] - m[45]) / f[1]);
	
	// Random Number (0 to 1)
	f[5] = m[41] / m[20];
	
	// Check If New Path Should Be Accepted
	if (f[4] < f[5])
		continue;
	
	// Swap Cities Back
	m[43] = m[m[30] + m[41]];            // City 1 Number
	m[m[30] + m[41]] = m[m[30] + m[42]]; // Swap City 1
	m[m[30] + m[42]] = m[43];            // Swap City 2
}

// Best Solution To Date = 10628
verify (m[200] < 11000); 