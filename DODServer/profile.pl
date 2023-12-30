
use vars qw(@DSA @DCA @SRM @All_Services);
my @struct=(
         ["key","description","name","default","type"]
        );

@DSA = (
		["SYSTEM\\CurrentControlSet\\Services\\DSA","Image Path","ImagePath","%EXEDIR%\\DSA.exe","S"],
               );
              
@DCA = (
		["SYSTEM\\CurrentControlSet\\Services\\DCA","Image Path","ImagePath","%EXEDIR%\\DCA.exe","S"],
               );            
               
               
@SRM = (
		["SYSTEM\\CurrentControlSet\\Services\\SRM","Image Path","ImagePath","%EXEDIR%\\SRM.exe","S"],
               );      
               
@All_Services = (
                 DSA,
                 DCA,
                 SRM,
                 );