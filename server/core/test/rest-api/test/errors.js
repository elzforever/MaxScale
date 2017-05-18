require("../utils.js")()


describe("Errors", function()
{
    before(startMaxScale)

    it("error on invalid PUT request", function()
    {
        return request.put(base_url + "/servers/server1", { json: {this_is: "a test"}})
               .should.be.rejected
    })

    it("error on invalid POST request", function()
    {
        return request.post(base_url + "/servers", { json: {this_is: "a test"}})
               .should.be.rejected
    })

    after(stopMaxScale)
});
