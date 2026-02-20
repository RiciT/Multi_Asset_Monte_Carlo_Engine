using Downloads, CSV, DataFrames, Statistics, Dates, LinearAlgebra

# find some more interesting ones later but now sticking with FAANG
tickers = ["AAPL", "MSFT", "NVDA", "GOOGL", "AMZN", "META"]

stopTime = Int(floor(datetime2unix(now())))
startTime = Int(floor(datetime2unix(now() - Year(1))))

function fetchYahooData(ticker, t1, t2)
    url = "https://query1.finance.yahoo.com/v7/finance/download/$ticker?period1=$t1&period2=$t2&interval=1d&events=history&includeAdjustedClose=true"
    
    io = IOBuffer()
    Downloads.download(url, io, headers = ["User-Agent" => "Mozilla/5.0"])
    
    seekstart(io)
    return CSV.read(io, DataFrame)
end

try
    # init with first iter
    pricesDf = fetchYahooData(tickers[1], startTime, stopTime)
    pricesDf = pricesDf[:, [:Date, Symbol("Adj Close")]]
    rename!(pricesDf, Symbol("Adj Close") => Symbol(tickers[1]))

    for i in 2:length(tickers)
        currentTicker = tickers[i]
        
        tempDf = fetchYahooData(currentTicker, startTime, stopTime)
        tempDf = tempDf[:, [:Date, Symbol("Adj Close")]]
        rename!(tempDf, Symbol("Adj Close") => Symbol(currentTicker))
        
        pricesDf = innerjoin(pricesDf, tempDf, on=:Date)
    end

    sort!(pricesDf, :Date)
    
    priceCols = Symbol.(tickers)
    priceMatrix = Matrix{Float64}(pricesDf[:, priceCols])
    
    returns = diff(log.(priceMatrix), dims=1)
    
    vols = std(returns, dims=1) .* sqrt(252)
    
    corrMatrix = cor(returns)
    corrMatrix += I * 1e-8
    
    spots = priceMatrix[end, :]

    CSV.write("market_vols.csv", DataFrame(vols = vec(vols)), header=false)
    CSV.write("market_spots.csv", DataFrame(spots = vec(spots)), header=false)
    CSV.write("market_correlation.csv", Tables.table(corrMatrix), header=false)

catch e
    @error "Data fetch failed." exception=(e, catch_backtrace())
end
