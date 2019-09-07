var show=true;
var obj=null;

$(document).ready(function(){
	chrome.storage.local.get({"image":"","opacity":0},function(result)
	{
		obj=$("<div style='position:fixed;z-index:999999;top:0px;left:0px;width:100%;height:100%;background:url("+result.image+") no-repeat center center fixed;-webkit-background-size:100% 100%;-moz-background-size:contain;-o-background-size:contain;background-size:cover;opacity:"+(result.opacity/100).toString()+";pointer-events: none;'></div>");
		$("body").prepend(obj);
		show=true;
	});
	
	chrome.runtime.onMessage.addListener(
	function(request, sender, sendResponse) {
			if(request.action=="query")
				sendResponse({show:show});
			if(request.action=="refresh")
				chrome.storage.local.get({"image":"","opacity":0},function(result)
				{
					obj.remove();
					obj=$("<div style='position:fixed;z-index:999999;top:0px;left:0px;width:100%;height:100%;background:url("+result.image+") no-repeat center center fixed;-webkit-background-size:100% 100%;-moz-background-size:contain;-o-background-size:contain;background-size:cover;opacity:"+(result.opacity/100).toString()+";pointer-events: none;'></div>");
					if(show)
						$("body").prepend(obj);
				});
			if(request.show==false){
				obj.detach();
				show=false;
			}
			if(request.show==true){
				$("body").prepend(obj);
				show=true;
			}
			//sendResponse({ok: "ok"});
		}
	);
});
