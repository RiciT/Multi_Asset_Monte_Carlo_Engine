using Downloads, CSV, DataFrames, Statistics, Dates, LinearAlgebra

# find some more interesting ones later but now sticking with FAANG
tickers = ["AAPL", "MSFT", "NVDA", "GOOGL", "AMZN", "META"]

function fetchStooqData(ticker)
    url = "https://stooq.com/q/d/l/?s=$(lowercase(ticker)).us&i=d";
    io = IOBuffer();
    Downloads.download(url, io);
    seekstart(io);
    return CSV.read(io, DataFrame);
end

try
    local pricesDf;
    pricesDf = fetchStooqData(tickers[1]);
    pricesDf = pricesDf[:, [:Date, :Close]];
    rename!(pricesDf, :Close => Symbol(tickers[1]));

    for i in 2:length(tickers)
        currentTicker = tickers[i];
        
        tempDf = fetchStooqData(currentTicker);
        tempDf = tempDf[:, [:Date, :Close]];
        rename!(tempDf, :Close => Symbol(currentTicker));
        
        pricesDf = innerjoin(pricesDf, tempDf, on=:Date);
    end

    sort!(pricesDf, :Date);
    pricesDf = last(pricesDf, 252);
    
    priceCols = Symbol.(tickers);
    priceMatrix = Matrix{Float64}(pricesDf[:, priceCols]);
    
    returns = diff(log.(priceMatrix), dims=1);
    vols = std(returns, dims=1) .* sqrt(252);
    
    corrMatrix = cor(returns);
    
    spots = priceMatrix[end, :];

    CSV.write("../data/market_vols.csv", DataFrame(vols = vec(vols)), header=false);
    CSV.write("../data/market_spots.csv", DataFrame(spots = vec(spots)), header=false);
    CSV.write("../data/market_correlation.csv", Tables.table(corrMatrix), header=false);

    println("done.");

catch e
    @error "data fetch failed" exception=(e, catch_backtrace());
end
