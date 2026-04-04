{ config
,  options
,  lib
,  pkgs
,  ... }:

let
  cfg = config.services.wbg-iotd;
  wbg-iotd-bin = lib.getExe cfg.package;
in
{
  options.services.wbg-iotd = {
    enable = mkEnableOption "wbg-iotd";
    package = mkPackageOption pkgs "wbg-iotd" { };
  };

  systemd.user.services = lib.mkIf cfg.enable {
    wbg-iotd = {
      Unit = {
        Description = "Wallpaper image-of-the-day manager.";
        PartOf = [ "graphical-session.target" ];
        After = [ "graphical-session.target" ];
      };
      Service = {
        Type = "exec";
        ExecStart = "${wbg-iotd-bin}";
        Restart = "always";
        RestartSec = 10;
      };
      Install = {
        WantedBy = [ "graphical-session.target" ];
      };
    };
  };
}
