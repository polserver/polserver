use uo;
use os;
function mapinfo_test()
  syslog("mapinfo_test..");
  var success:=1;
  
  for x:=0 to 191
    for y:=0 to 191
      var info:=GetMapInfo(x,y);
      if ((x>23 && x<33) || (x>(192-34) && x<(192-23))
       || (y>23 && y<33) || (y>(192-34) && y<(192-23)))
        continue; // border is a bit weird bug or feature?
      endif
      if (x<30 || x>(192-30) || y<30 || y>(192-30))
        if (info.landtile != 0xa8 && info.z != -5)
          syslog("Wrong mapinfo {},{}: expected {},{} got {},{}".format(x,y,0xa8,-5,info.landtile,info.z));
          success:=0;
        endif
      else
        if (info.landtile != 0x3 && info.z != 0)
          syslog("Wrong mapinfo {},{}: expected {},{} got {},{}".format(x,y,0x3,0,info.landtile,info.z));
          success:=0;
        endif
      endif
    endfor
  endfor
  return success;
endfunction

function statics_test()
  syslog("statics_test..");
  var success:=1;

  var info:=ListStaticsInBox(0,0,-128,10000,10000,128);
  if (info[1].objtype !=0x3fff || info[1].x !=10 || info[1].y!=5 || info[1].z!=-5)
    syslog("Wrong static found at pos 1 :"+info[1]);
    success:=0;
  endif
  if (info[2].objtype !=0x3fff || info[2].x !=11 || info[2].y!=5 || info[2].z!=10)
    syslog("Wrong static found at pos 2 :"+info[2]);
    success:=0;
  endif
  return success;
endfunction
