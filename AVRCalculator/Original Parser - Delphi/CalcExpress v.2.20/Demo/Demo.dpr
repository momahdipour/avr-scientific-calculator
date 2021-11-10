program Demo;

uses
  Forms,
  Main in 'Main.pas' {DemoForm},
  CalcExpress in 'CalcExpress.pas';

{$R *.RES}

begin
  Application.Initialize;
  Application.CreateForm(TDemoForm, DemoForm);
  Application.Run;
end.
