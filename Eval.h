#pragma once
#include "Typedefs.h"
namespace hce {
    constexpr int PHASE_PIECE_VALUES[5] = {4,2,1,1,0};
    constexpr const static eval_t MATE_VALUE = 1048576;
    constexpr uint64_t king_zone_attacks[] = {111669149728ULL, 227633332214ULL, 193273593849ULL, 94489346038ULL, 64424574959ULL};
    constexpr uint64_t king_psts[] = {249108168604ULL, 339302481872ULL, 281264523378672ULL, 280989645471742ULL, 280933810831378ULL, 280637458087975ULL, 280856501420044ULL, 281221573705616ULL, 450971631548ULL, 167503790073ULL, 47244640267ULL, 281131379327012ULL, 281071249784895ULL, 98784247868ULL, 281324652855349ULL, 281264523378645ULL, 219043397601ULL, 68719476750ULL, 281217278672935ULL, 281006825275458ULL, 281015415210068ULL, 281135674294359ULL, 281032595079240ULL, 281466386841571ULL, 281049775013884ULL, 281307472986143ULL, 281152854163513ULL, 280830731616344ULL, 280723357433961ULL, 281062659850341ULL, 249108103212ULL, 280744832270376ULL, 281457796906970ULL, 281324652855331ULL, 281225868607542ULL, 280869386322003ULL, 280826436649060ULL, 281316062920805ULL, 281436322005066ULL, 281058364882957ULL, 281225868673017ULL, 281432027037717ULL, 281217278672936ULL, 281097019588668ULL, 281032595079257ULL, 292057776216ULL, 281410552201309ULL, 281290293116947ULL, 274877972430ULL, 257698103289ULL, 281462091808782ULL, 281002530308144ULL, 280968170569809ULL, 25769803869ULL, 154618822736ULL, 281466386776078ULL, 253403135900ULL, 231928299484ULL, 281157149130757ULL, 280607393316901ULL, 280594508415031ULL, 281174328999986ULL, 281178623967273ULL, 300647776126ULL};
    constexpr uint64_t piece_type_psts[][64] = {
            {6025839117914ULL, 6034429052512ULL, 6025839117934ULL, 6064493823626ULL, 6021544150680ULL, 6154688136824ULL, 6107443496542ULL, 5828270622333ULL, 5944234739305ULL, 6073083758186ULL, 6090263627399ULL, 6017249183405ULL, 6073083758259ULL, 6137508267662ULL, 5978594477715ULL, 5905580033663ULL, 6004364281465ULL, 6141803234924ULL, 6068788790949ULL, 6008659248825ULL, 6060198856377ULL, 6150393169602ULL, 5987184412353ULL, 6111738463889ULL, 6116033431155ULL, 6150393169541ULL, 6034429052588ULL, 6090263627462ULL, 6025839118037ULL, 6163278071482ULL, 5952824674000ULL, 6279242188405ULL, 6055903889013ULL, 6137508267659ULL, 6081673692854ULL, 6081673692870ULL, 6081673692876ULL, 6189047875257ULL, 5866925328105ULL, 6171868006014ULL, 5965709575772ULL, 6210522711627ULL, 6111738463898ULL, 6055903889074ULL, 6124623365812ULL, 6343666697875ULL, 6137508267682ULL, 6249177417325ULL, 6111738463785ULL, 6240587482651ULL, 6184752907888ULL, 6137508267669ULL, 6137508267694ULL, 6360846567020ULL, 6051608921748ULL, 6395206305334ULL, 6081673692705ULL, 6317896893922ULL, 6137508267614ULL, 6141803234941ULL, 6124623365767ULL, 6270652253787ULL, 6365141534318ULL, 6060198856288ULL},
            {2645699855290ULL, 2516850836413ULL, 2534030705605ULL, 2585570313162ULL, 2718714299347ULL, 2808908612558ULL, 2856153252817ULL, 2903397893077ULL, 2654289789892ULL, 2602750182338ULL, 2594160247747ULL, 2589865280466ULL, 2808908612561ULL, 2954937500627ULL, 2817498547173ULL, 2804613645283ULL, 2727304233935ULL, 2710124364737ULL, 2658584757186ULL, 2662879724501ULL, 2830383449054ULL, 2967822402517ULL, 2924872729582ULL, 2817498547187ULL, 2761663972299ULL, 2688649528260ULL, 2654289789896ULL, 2757369005009ULL, 2890512991191ULL, 2972117369809ULL, 3036541879265ULL, 2800318677999ULL, 2787433776063ULL, 2710124364726ULL, 2684354560961ULL, 2761663972296ULL, 2911987827648ULL, 3161095930812ULL, 2899102925796ULL, 2877628089314ULL, 2710124364726ULL, 2697239462829ULL, 2607045149621ULL, 2546915607498ULL, 2838973383611ULL, 3027951944635ULL, 3027951944656ULL, 2997887173586ULL, 2770253906866ULL, 2783138808735ULL, 2800318677911ULL, 2684354560951ULL, 2834678416313ULL, 3242700309425ULL, 2954937500618ULL, 2920577762260ULL, 2637109920685ULL, 2607045149610ULL, 2688649528217ULL, 2654289789871ULL, 2860448220080ULL, 3053721748396ULL, 3165390898100ULL, 3096671421385ULL},
            {2057289335294ULL, 2177548419609ULL, 2156073583136ULL, 2126008812065ULL, 2156073583145ULL, 2216203125297ULL, 2126008812036ULL, 2048699400727ULL, 2177548419608ULL, 2224793059862ULL, 2207613190705ULL, 2151778615871ULL, 2263447765569ULL, 2375116915239ULL, 2349347111455ULL, 1924145349158ULL, 2104533975548ULL, 2289217569300ULL, 2241972929083ULL, 2199023256137ULL, 2323577307707ULL, 2379411882551ULL, 2272037700135ULL, 1962800054820ULL, 2044404433436ULL, 2168958485037ULL, 2241972929085ULL, 2353642078786ULL, 2366526980687ULL, 2422361555497ULL, 2168958485033ULL, 1679332213305ULL, 2087354106388ULL, 2220498092592ULL, 2280627634755ULL, 2319282340419ULL, 2370821947968ULL, 2297807503922ULL, 2237677961760ULL, 1644972474930ULL, 2074469204504ULL, 2314987373083ULL, 2250562863676ULL, 2272037700159ULL, 2332167242303ULL, 2473901163061ULL, 2220498092573ULL, 1765231559204ULL, 2229088027137ULL, 2345052144160ULL, 2259152798242ULL, 2207613190711ULL, 2263447765560ULL, 2319282340395ULL, 2134598746669ULL, 1971389989401ULL, 2160368550373ULL, 2246267896313ULL, 2284922601999ULL, 2177548419598ULL, 2061584302635ULL, 2190433321517ULL, 2070174237191ULL, 1808181232149ULL},
            {1387274437079ULL, 1679332213222ULL, 1769526526452ULL, 1902670512653ULL, 1997159793161ULL, 2014339662330ULL, 1902670512608ULL, 953482740119ULL, 1743756722630ULL, 1760936591870ULL, 1932735283729ULL, 2018634629661ULL, 2083059139116ULL, 2263447765522ULL, 2044404433409ULL, 1202590843373ULL, 1662152344050ULL, 1876900708876ULL, 2022929596960ULL, 2121713844802ULL, 2254857830976ULL, 2379411882541ULL, 2207613190671ULL, 1653562409477ULL, 1765231559159ULL, 1967095022098ULL, 2052994368055ULL, 2126008812099ULL, 2409476653634ULL, 2409476653618ULL, 2289217569300ULL, 1842540970489ULL, 1795296330231ULL, 1967095022095ULL, 2121713844788ULL, 2203318223429ULL, 2181843386959ULL, 2611340116510ULL, 2160368550408ULL, 2065879269886ULL, 1894080578021ULL, 1992864825863ULL, 2040109466137ULL, 2138893713976ULL, 2400886719041ULL, 2654289789460ULL, 2559800508913ULL, 1657857376731ULL, 1769526526418ULL, 1876900708847ULL, 2057289335305ULL, 2121713844770ULL, 2027224564280ULL, 2319282340361ULL, 2022929596924ULL, 1378684502515ULL, 1589137899976ULL, 1868310774264ULL, 1885490643447ULL, 1967095022078ULL, 2168958484996ULL, 2104533975537ULL, 2117418877387ULL, 1305670058354ULL},
            {0ULL, 244813136078ULL, 244813136070ULL, 257698037968ULL, 335007449336ULL, 450971566437ULL, 1052266987964ULL, 0ULL, 0ULL, 420906795215ULL, 420906795208ULL, 450971566284ULL, 506806141160ULL, 554050781556ULL, 1189705941424ULL, 0ULL, 0ULL, 386547056825ULL, 412316860590ULL, 442381631665ULL, 528280977610ULL, 768799146303ULL, 1052266987954ULL, 0ULL, 0ULL, 317827580100ULL, 420906795201ULL, 545460846765ULL, 549755814077ULL, 807453851937ULL, 1258425418087ULL, 0ULL, 0ULL, 455266533580ULL, 511101108407ULL, 558345748650ULL, 682899800241ULL, 807453851929ULL, 1120986464617ULL, 0ULL, 0ULL, 575525617847ULL, 442381631666ULL, 489626271918ULL, 639950127287ULL, 983547511039ULL, 1000727380341ULL, 0ULL, 0ULL, 721554505910ULL, 639950127290ULL, 549755814079ULL, 648540061907ULL, 811748819271ULL, 446676599231ULL, 0ULL, 0ULL, 390842024109ULL, 450971566250ULL, 399431958705ULL, 485331304657ULL, 519691043137ULL, 274877907408ULL, 0ULL},
    };


    inline eval_t getEvalFromPacked (packed_eval_t packed, int phase) { // higher phase means closer to mg
        int mg = int(int16_t(packed >> 32 & 0xffff));
        int eg = int(int16_t(packed & 0xffff));
        return float(mg * phase + eg * (24 - phase)) / 24.0F;
    }
};