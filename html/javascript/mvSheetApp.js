var ref = new Wilddog("https://lmagic.wilddogio.com/iqiyi/");
var app = new Vue({
    el:"#app",
    data:{
        sheetname:"",
        sheet:[],
        sheetNum:0
    },
    methods:{
        getSheetByYeGou:function(){
            userLocalStorage = window.localStorage
            var tempSheet = this.sheet
            var that = this
            if(userLocalStorage.getItem('username') != null){
                var loginname = userLocalStorage.getItem('username');
                that.sheetname = userLocalStorage.getItem('sheetname');
                ref.child(loginname).child("mvSheet").child(this.sheetname).on("value",function(snapshot){
                    snapshot.forEach(function(snap){
                        if(JSON.stringify(tempSheet).indexOf(JSON.stringify(snap.val())) == -1 && snap.val() != ""){
                            tempSheet.push(snap.val());
                        }
                    });
                    that.sheetNum = tempSheet.length;
                });
            }
            this.sheet = tempSheet
            $(".collectA").height($(".collectA").width()*124/220);
            $(".oneCollect").height($(".collectA").height()+45);
        },
        animateIcon:function(event){
            $(event.target).find(".icondiv").css('bottom', "-20px");
            $(event.target).find(".collectTitle").css('color', "#77a610");
            $(event.target).find(".fa-trash").css("visibility","visible");
        },
        stopAnimateIcon:function(event){
            $(event.target).find(".icondiv").css('bottom', "-25px");
            $(event.target).find(".collectTitle").css('color', "#000");
            $(event.target).find(".fa-trash").css("visibility","hidden");
        },
        getLink:function(x){
            return 'iqiyi://mobile/player?aid=' + x.aid + '&tvid=' + x.tvid + '&ftype=27&to=3&url=' + encodeURIComponent(window.location.href);
        },
        removeSheetCollect:function(x){
            var loginname = userLocalStorage.getItem('username');
            ref.child(loginname).child("mvSheet").child(this.sheetname).child(x.aid).remove();
            this.sheet.splice(this.sheet.indexOf(x),1);
            this.sheetNum--;
        }
    },
    created:function(){
        this.getSheetByYeGou()
    }
})
